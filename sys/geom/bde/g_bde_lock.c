/*-
 * Copyright (c) 2002 Poul-Henning Kamp
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by Poul-Henning Kamp
 * and NAI Labs, the Security Research Division of Network Associates, Inc.
 * under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"), as part of the
 * DARPA CHATS research program.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 *
 * This souce file contains routines which operates on the lock sectors, both
 * for the kernel and the userland program gbde(1).
 *
 */

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/stdint.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/md5.h>

#ifdef _KERNEL
#include <sys/malloc.h>
#include <sys/systm.h>
#else
#include <err.h>
#define CTASSERT(foo)
#define KASSERT(foo, bar) do { if(!(foo)) { warn bar ; exit (1); } } while (0)
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define g_free(foo)	free(foo)
#endif

#include <crypto/rijndael/rijndael.h>
#include <crypto/sha2/sha2.h>

#include <geom/geom.h>
#include <geom/bde/g_bde.h>

/*
 * Hash the raw pass-phrase.
 *
 * Security objectives: produce from the pass-phrase a fixed length
 * bytesequence with PRN like properties in a reproducible way retaining
 * as much entropy from the pass-phrase as possible.
 *
 * SHA2-512 makes this easy.
 */

void
g_bde_hash_pass(struct g_bde_softc *sc, const void *input, u_int len)
{
	SHA512_CTX cx;

	SHA512_Init(&cx);
	SHA512_Update(&cx, input, len);
	SHA512_Final(sc->sha2, &cx);
}

/*
 * Encode/Decode the lock structure in byte-sequence format.
 *
 * Security objectives: Store in pass-phrase dependent variant format.
 *
 * C-structure packing and byte-endianess depends on architecture, compiler
 * and compiler options.  Writing raw structures to disk is therefore a bad
 * idea in these enlightend days.
 *
 * We spend a fraction of the key-material on shuffling the fields around
 * so they will be stored in an unpredictable sequence.
 *
 * For each byte of the key-material we derive two field indexes, and swap
 * the position of those two fields.
 *
 * I have not worked out the statistical properties of this shuffle, but
 * given that the key-material has PRN properties, the primary objective
 * of making it hard to figure out which bits are where in the lock sector
 * is sufficiently fulfilled.
 *
 * We include (and shuffle) an extra hash field in the stored version for
 * identification and versioning purposes.  This field contains the MD5 hash
 * of a version identifier (currently "0000") followed by the stored lock
 * sector byte-sequence substituting zero bytes for the hash field.
 *
 * The stored keysequence is protected by AES/256/CBC elsewhere in the code
 * so the fact that the generated byte sequence has a much higher than
 * average density of zero bits (from the numeric fields) is not currently
 * a concern.
 *
 * Should this later become a concern, a simple software update and 
 * pass-phrase change can remedy the situation.  One possible solution 
 * could be to XOR the numeric fields with a key-material derived PRN.
 *
 * The chosen shuffle algorithm only works as long as we have no more than 16 
 * fields in the stored part of the lock structure (hence the CTASSERT below).
 */

CTASSERT(NLOCK_FIELDS <= 16);

static void
g_bde_shuffle_lock(struct g_bde_softc *sc, int *buf)
{
	int j, k, l;
	u_int u;

	/* Assign the fields sequential positions */
	for(u = 0; u < NLOCK_FIELDS; u++)
		buf[u] = u;

	/* Then mix it all up */
	for(u = 48; u < sizeof(sc->sha2); u++) {
		j = sc->sha2[u] % NLOCK_FIELDS;
		k = (sc->sha2[u] / NLOCK_FIELDS) % NLOCK_FIELDS;
		l = buf[j];
		buf[j] = buf[k];
		buf[k] = l;
	}
}

int
g_bde_encode_lock(struct g_bde_softc *sc, struct g_bde_key *gl, u_char *ptr)
{
	int shuffle[NLOCK_FIELDS];
	u_char *hash, *p;
	int i;
	MD5_CTX c;

	p = ptr;
	hash = NULL;
	g_bde_shuffle_lock(sc, shuffle);
	for (i = 0; i < NLOCK_FIELDS; i++) {
		switch(shuffle[i]) {
		case 0:
			g_enc_le8(p, gl->sector0);
			p += 8;
			break;
		case 1:
			g_enc_le8(p, gl->sectorN);
			p += 8;
			break;
		case 2:
			g_enc_le8(p, gl->keyoffset);
			p += 8;
			break;
		case 3:
			g_enc_le4(p, gl->sectorsize);
			p += 4;
			break;
		case 4:
			g_enc_le4(p, gl->flags);
			p += 4;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			g_enc_le8(p, gl->lsector[shuffle[i] - 5]);
			p += 8;
			break;
		case 9:
			bcopy(gl->spare, p, sizeof gl->spare);
			p += sizeof gl->spare;
			break;
		case 10:
			bcopy(gl->salt, p, sizeof gl->salt);
			p += sizeof gl->salt;
			break;
		case 11:
			bcopy(gl->mkey, p, sizeof gl->mkey);
			p += sizeof gl->mkey;
			break;
		case 12:
			bzero(p, 16);
			hash = p;
			p += 16;
			break;
		}
	}
	if(ptr + G_BDE_LOCKSIZE != p)
		return(-1);
	if (hash == NULL)
		return(-1);
	MD5Init(&c);
	MD5Update(&c, "0000", 4);	/* Versioning */
	MD5Update(&c, ptr, G_BDE_LOCKSIZE);
	MD5Final(hash, &c);
	return(0);
}

int
g_bde_decode_lock(struct g_bde_softc *sc, struct g_bde_key *gl, u_char *ptr)
{
	int shuffle[NLOCK_FIELDS];
	u_char *p;
	u_char hash[16], hash2[16];
	MD5_CTX c;
	int i;

	p = ptr;
	g_bde_shuffle_lock(sc, shuffle);
	for (i = 0; i < NLOCK_FIELDS; i++) {
		switch(shuffle[i]) {
		case 0:
			gl->sector0 = g_dec_le8(p);
			p += 8;
			break;
		case 1:
			gl->sectorN = g_dec_le8(p);
			p += 8;
			break;
		case 2:
			gl->keyoffset = g_dec_le8(p);
			p += 8;
			break;
		case 3:
			gl->sectorsize = g_dec_le4(p);
			p += 4;
			break;
		case 4:
			gl->flags = g_dec_le4(p);
			p += 4;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			gl->lsector[shuffle[i] - 5] = g_dec_le8(p);
			p += 8;
			break;
		case 9:
			bcopy(p, gl->spare, sizeof gl->spare);
			p += sizeof gl->spare;
			break;
		case 10:
			bcopy(p, gl->salt, sizeof gl->salt);
			p += sizeof gl->salt;
			break;
		case 11:
			bcopy(p, gl->mkey, sizeof gl->mkey);
			p += sizeof gl->mkey;
			break;
		case 12:
			bcopy(p, hash2, sizeof hash2);
			bzero(p, sizeof hash2);
			p += sizeof hash2;
			break;
		}
	}
	if(ptr + G_BDE_LOCKSIZE != p)
		return(-1);
	MD5Init(&c);
	MD5Update(&c, "0000", 4);	/* Versioning */
	MD5Update(&c, ptr, G_BDE_LOCKSIZE);
	MD5Final(hash, &c);
	if (bcmp(hash, hash2, sizeof hash2))
		return (1);
	return (0);
}

/*
 * Encode/Decode the locksector address ("metadata") with key-material.
 *
 * Security objectives: Encode/Decode the metadata encrypted by key-material.
 *
 * A simple AES/128/CBC will do.  We take care to always store the metadata
 * in the same endianess to make it MI.
 *
 * In the typical case the metadata is stored in encrypted format in sector
 * zero on the media, but at the users discretion or if the piece of the
 * device used (sector0...sectorN) does not contain sector zero, it can
 * be stored in a filesystem or on a PostIt.
 *
 * The inability to easily locate the lock sectors makes an attack on a
 * cold disk much less attractive, without unduly inconveniencing the
 * legitimate user who can feasibly do a brute-force scan if the metadata
 * was lost.
 */

int
g_bde_keyloc_encrypt(struct g_bde_softc *sc, uint64_t *input, void *output)
{
	u_char buf[16];
	keyInstance ki;
	cipherInstance ci;

	g_enc_le8(buf, input[0]);
	g_enc_le8(buf + 8, input[1]);
	AES_init(&ci);
	AES_makekey(&ki, DIR_ENCRYPT, G_BDE_KKEYBITS, sc->sha2 + 0);
	AES_encrypt(&ci, &ki, buf, output, sizeof buf);
	bzero(buf, sizeof buf);
	bzero(&ci, sizeof ci);
	bzero(&ki, sizeof ki);
	return (0);
}

int
g_bde_keyloc_decrypt(struct g_bde_softc *sc, void *input, uint64_t *output)
{
	keyInstance ki;
	cipherInstance ci;
	u_char buf[16];

	AES_init(&ci);
	AES_makekey(&ki, DIR_DECRYPT, G_BDE_KKEYBITS, sc->sha2 + 0);
	AES_decrypt(&ci, &ki, input, buf, sizeof buf);
	output[0] = g_dec_le8(buf);
	output[1] = g_dec_le8(buf + 8);
	bzero(buf, sizeof buf);
	bzero(&ci, sizeof ci);
	bzero(&ki, sizeof ki);
	return (0);
}

/*
 * Find and Encode/Decode lock sectors.
 *
 * Security objective: given the pass-phrase, find, decrypt, decode and
 * validate the lock sector contents.
 *
 * For ondisk metadata we cannot know beforehand which of the lock sectors
 * a given pass-phrase opens so we must try each of the metadata copies in
 * sector zero in turn.  If metadata was passed as an argument, we don't
 * have this problem.
 *
 */

static int
g_bde_decrypt_lockx(struct g_bde_softc *sc, u_char *meta, off_t mediasize, u_int sectorsize, u_int *nkey)
{
	u_char *buf, *q;
	struct g_bde_key *gl;
	uint64_t off[2];
	int error, m, i;
	keyInstance ki;
	cipherInstance ci;

	gl = &sc->key;

	/* Try to decrypt the metadata */
	error = g_bde_keyloc_decrypt(sc, meta, off);
	if (error)
		return(error);

	/* loose the random part */
	off[1] = 0;

	/* If it points ito thin blue air, forget it */
	if (off[0] + G_BDE_LOCKSIZE > (uint64_t)mediasize) {
		off[0] = 0;
		return (EINVAL);
	}

	/* The lock data may span two physical sectors. */

	m = 1;
	if (off[0] % sectorsize > sectorsize - G_BDE_LOCKSIZE)
		m++;

	/* Read the suspected sector(s) */
	buf = g_read_data(sc->consumer,
		off[0] - (off[0] % sectorsize),
		m * sectorsize, &error);
	if (buf == NULL) {
		off[0] = 0;
		return(error);
	}

	/* Find the byte-offset of the stored byte sequence */
	q = buf + off[0] % sectorsize;

	/* If it is all zero, somebody nuked our lock sector */
	for (i = 0; i < G_BDE_LOCKSIZE; i++)
		off[1] += q[i];
	if (off[1] == 0) {
		off[0] = 0;
		g_free(buf);
		return (ESRCH);
	}

	/* Decrypt the byte-sequence in place */
	AES_init(&ci);
	AES_makekey(&ki, DIR_DECRYPT, 256, sc->sha2 + 16);
	AES_decrypt(&ci, &ki, q, q, G_BDE_LOCKSIZE);
	
	/* Decode the byte-sequence */
	i = g_bde_decode_lock(sc, gl, q);
	q = NULL;
	if (i < 0) {
		off[0] = 0;
		return (EDOOFUS);	/* Programming error */
	} else if (i > 0) {
		off[0] = 0;
		return (ENOTDIR);	/* Hash didn't match */
	}

	bzero(buf, sectorsize * m);
	g_free(buf);

	/* If the masterkey is all zeros, user destroyed it */
	off[1] = 0;
	for (i = 0; i < (int)sizeof(gl->mkey); i++)
		off[1] += gl->mkey[i];
	if (off[1] == 0)
		return (ENOENT);

	/* Finally, find out which key was used by matching the byte offset */
	for (i = 0; i < G_BDE_MAXKEYS; i++)
		if (nkey != NULL && off[0] == gl->lsector[i])
			*nkey = i;
	off[0] = 0;
	return (0);
}

int
g_bde_decrypt_lock(struct g_bde_softc *sc, u_char *keymat, u_char *meta, off_t mediasize, u_int sectorsize, u_int *nkey)
{
	u_char *buf, buf1[16];
	int error, e, i;

	/* set up the key-material */
	bcopy(keymat, sc->sha2, SHA512_DIGEST_LENGTH);

	/* If passed-in metadata is non-zero, use it */
	bzero(buf1, sizeof buf1);
	if (bcmp(buf1, meta, sizeof buf1))
		return (g_bde_decrypt_lockx(sc, meta, mediasize,
		    sectorsize, nkey));

	/* Read sector zero */
	buf = g_read_data(sc->consumer, 0, sectorsize, &error);
	if (buf == NULL)
		return(error);

	/* Try each index in turn, save indicative errors for final result */
	error = EINVAL;
	for (i = 0; i < G_BDE_MAXKEYS; i++) {
		e = g_bde_decrypt_lockx(sc, buf + i * 16, mediasize,
		    sectorsize, nkey);
		/* Success or destroyed master key terminates */
		if (e == 0 || e == ENOENT) {
			error = e;
			break;
		}
		if (e != 0 && error == EINVAL)
			error = e;
	}
	g_free(buf);
	return (error);
}
