/*
 * 
 * keys.h
 *
 * priv key definitions
 *
 * a Net::DNS like library for C
 *
 * (c) NLnet Labs, 2005-2006
 *
 * See the file LICENSE for the license
 */

/**
 * \file
 *
 * Addendum to \ref dnssec.h, this module contains key and algorithm definitions and functions.
 */
 

#ifndef LDNS_KEYS_H
#define LDNS_KEYS_H

#include <ldns/common.h>
#if LDNS_BUILD_CONFIG_HAVE_SSL
#include <openssl/ssl.h>
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */
#include <ldns/util.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

extern ldns_lookup_table ldns_signing_algorithms[];

#define LDNS_KEY_ZONE_KEY 0x0100   /* rfc 4034 */
#define LDNS_KEY_SEP_KEY 0x0001    /* rfc 4034 */
#define LDNS_KEY_REVOKE_KEY 0x0080 /* rfc 5011 */

/**
 * Algorithms used in dns
 */
enum ldns_enum_algorithm
{
        LDNS_RSAMD5             = 1,   /* RFC 4034,4035 */
        LDNS_DH                 = 2,
        LDNS_DSA                = 3,
        LDNS_ECC                = 4,
        LDNS_RSASHA1            = 5,
        LDNS_DSA_NSEC3          = 6,
        LDNS_RSASHA1_NSEC3      = 7,
        LDNS_RSASHA256          = 8,   /* RFC 5702 */
        LDNS_RSASHA512          = 10,  /* RFC 5702 */
        LDNS_ECC_GOST           = 12,  /* RFC 5933 */
        LDNS_ECDSAP256SHA256    = 13,  /* RFC 6605 */
        LDNS_ECDSAP384SHA384    = 14,  /* RFC 6605 */
        LDNS_INDIRECT           = 252,
        LDNS_PRIVATEDNS         = 253,
        LDNS_PRIVATEOID         = 254
};
typedef enum ldns_enum_algorithm ldns_algorithm;

/**
 * Hashing algorithms used in the DS record
 */
enum ldns_enum_hash
{
        LDNS_SHA1               = 1,  /* RFC 4034 */
        LDNS_SHA256             = 2,  /* RFC 4509 */
        LDNS_HASH_GOST          = 3,  /* RFC 5933 */
        LDNS_SHA384             = 4   /* RFC 6605 */
};
typedef enum ldns_enum_hash ldns_hash;

/**
 * Algorithms used in dns for signing
 */
enum ldns_enum_signing_algorithm
{
	LDNS_SIGN_RSAMD5	 = LDNS_RSAMD5,
	LDNS_SIGN_RSASHA1	 = LDNS_RSASHA1,
	LDNS_SIGN_DSA		 = LDNS_DSA,
	LDNS_SIGN_RSASHA1_NSEC3  = LDNS_RSASHA1_NSEC3,
	LDNS_SIGN_RSASHA256	 = LDNS_RSASHA256,
	LDNS_SIGN_RSASHA512	 = LDNS_RSASHA512,
	LDNS_SIGN_DSA_NSEC3	 = LDNS_DSA_NSEC3,
	LDNS_SIGN_ECC_GOST       = LDNS_ECC_GOST,
        LDNS_SIGN_ECDSAP256SHA256 = LDNS_ECDSAP256SHA256,
        LDNS_SIGN_ECDSAP384SHA384 = LDNS_ECDSAP384SHA384,
	LDNS_SIGN_HMACMD5	 = 157,	/* not official! This type is for TSIG, not DNSSEC */
	LDNS_SIGN_HMACSHA1	 = 158,	/* not official! This type is for TSIG, not DNSSEC */
	LDNS_SIGN_HMACSHA256 = 159  /* ditto */
};
typedef enum ldns_enum_signing_algorithm ldns_signing_algorithm;

/**
 * General key structure, can contain all types of keys that
 * are used in DNSSEC. Mostly used to store private keys, since
 * public keys can also be stored in a \ref ldns_rr with type
 * \ref LDNS_RR_TYPE_DNSKEY.
 *
 * This structure can also store some variables that influence the
 * signatures generated by signing with this key, for instance the
 * inception date.
 */
struct ldns_struct_key {
	ldns_signing_algorithm _alg;
	/** Whether to use this key when signing */
	bool _use;
	/** Storage pointers for the types of keys supported */
	/* TODO remove unions? */
	struct {
#if LDNS_BUILD_CONFIG_HAVE_SSL
#ifndef S_SPLINT_S
		/* The key can be an OpenSSL EVP Key
		 */
		EVP_PKEY *key;
#endif
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */
		/**
		 * The key can be an HMAC key
		 */
		struct {
			unsigned char *key;
			size_t size;
		} hmac;
		/** the key structure can also just point to some external
		 *  key data
		 */
		void *external_key;
	} _key;
	/** Depending on the key we can have extra data */
	union {
                /** Some values that influence generated signatures */
		struct {
			/** The TTL of the rrset that is currently signed */
			uint32_t orig_ttl;
			/** The inception date of signatures made with this key. */
			uint32_t inception;
			/** The expiration date of signatures made with this key. */
			uint32_t expiration;
			/** The keytag of this key. */
			uint16_t keytag;
			/** The dnssec key flags as specified in RFC4035, like ZSK and KSK */
			uint16_t flags;
		}  dnssec;
	} _extra;
	/** Owner name of the key */
	ldns_rdf *_pubkey_owner;
};
typedef struct ldns_struct_key ldns_key;

/**
 * Same as rr_list, but now for keys 
 */
struct ldns_struct_key_list
{
	size_t _key_count;
	ldns_key **_keys;
};
typedef struct ldns_struct_key_list ldns_key_list;


/**
 * Creates a new empty key list
 * \return a new ldns_key_list structure pointer
 */
ldns_key_list *ldns_key_list_new(void);

/** 
 * Creates a new empty key structure
 * \return a new ldns_key * structure
 */
ldns_key *ldns_key_new(void);

/**
 * Creates a new key based on the algorithm
 *
 * \param[in] a The algorithm to use
 * \param[in] size the number of bytes for the keysize
 * \return a new ldns_key structure with the key
 */
ldns_key *ldns_key_new_frm_algorithm(ldns_signing_algorithm a, uint16_t size);

/**
 * Creates a new priv key based on the 
 * contents of the file pointed by fp.
 *
 * The file should be in Private-key-format v1.x.
 *
 * \param[out] k the new ldns_key structure
 * \param[in] fp the file pointer to use
 * \return an error or LDNS_STATUS_OK
 */
ldns_status ldns_key_new_frm_fp(ldns_key **k, FILE *fp);

/**
 * Creates a new private key based on the 
 * contents of the file pointed by fp
 *
 * The file should be in Private-key-format v1.x.
 *
 * \param[out] k the new ldns_key structure
 * \param[in] fp the file pointer to use
 * \param[in] line_nr pointer to an integer containing the current line number (for debugging purposes)
 * \return an error or LDNS_STATUS_OK
 */
ldns_status ldns_key_new_frm_fp_l(ldns_key **k, FILE *fp, int *line_nr);

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * Read the key with the given id from the given engine and store it
 * in the given ldns_key structure. The algorithm type is set
 */
ldns_status ldns_key_new_frm_engine(ldns_key **key, ENGINE *e, char *key_id, ldns_algorithm);


/**
 * frm_fp helper function. This function parses the
 * remainder of the (RSA) priv. key file generated from bind9
 * \param[in] fp the file to parse
 * \return NULL on failure otherwise a RSA structure
 */
RSA *ldns_key_new_frm_fp_rsa(FILE *fp);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * frm_fp helper function. This function parses the
 * remainder of the (RSA) priv. key file generated from bind9
 * \param[in] fp the file to parse
 * \param[in] line_nr pointer to an integer containing the current line number (for debugging purposes)
 * \return NULL on failure otherwise a RSA structure
 */
RSA *ldns_key_new_frm_fp_rsa_l(FILE *fp, int *line_nr);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * frm_fp helper function. This function parses the
 * remainder of the (DSA) priv. key file
 * \param[in] fp the file to parse
 * \return NULL on failure otherwise a RSA structure
 */
DSA *ldns_key_new_frm_fp_dsa(FILE *fp);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * frm_fp helper function. This function parses the
 * remainder of the (DSA) priv. key file
 * \param[in] fp the file to parse
 * \param[in] line_nr pointer to an integer containing the current line number (for debugging purposes)
 * \return NULL on failure otherwise a RSA structure
 */
DSA *ldns_key_new_frm_fp_dsa_l(FILE *fp, int *line_nr);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * frm_fp helper function. This function parses the
 * remainder of the (HMAC-MD5) key file
 * This function allocated a buffer that needs to be freed
 * \param[in] fp the file to parse
 * \param[out] hmac_size the number of bits in the resulting buffer
 * \return NULL on failure otherwise a newly allocated char buffer
 */
unsigned char *ldns_key_new_frm_fp_hmac(FILE *fp, size_t *hmac_size);
#endif

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * frm_fp helper function. This function parses the
 * remainder of the (HMAC-MD5) key file
 * This function allocated a buffer that needs to be freed
 * \param[in] fp the file to parse
 * \param[in] line_nr pointer to an integer containing the current line number (for error reporting purposes)
 * \param[out] hmac_size the number of bits in the resulting buffer
 * \return NULL on failure otherwise a newly allocated char buffer
 */
unsigned char *ldns_key_new_frm_fp_hmac_l(FILE *fp, int *line_nr, size_t *hmac_size);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

/* acces write functions */
/**
 * Set the key's algorithm
 * \param[in] k the key
 * \param[in] l the algorithm
 */
void ldns_key_set_algorithm(ldns_key *k, ldns_signing_algorithm l);
#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * Set the key's evp key
 * \param[in] k the key
 * \param[in] e the evp key
 */
void ldns_key_set_evp_key(ldns_key *k, EVP_PKEY *e);

/**
 * Set the key's rsa data.
 * The rsa data should be freed by the user.
 * \param[in] k the key
 * \param[in] r the rsa data
 */
void ldns_key_set_rsa_key(ldns_key *k, RSA *r);

/**
 * Set the key's dsa data
 * The dsa data should be freed by the user.
 * \param[in] k the key
 * \param[in] d the dsa data
 */
void ldns_key_set_dsa_key(ldns_key *k, DSA *d);

/**
 * Assign the key's rsa data
 * The rsa data will be freed automatically when the key is freed.
 * \param[in] k the key
 * \param[in] r the rsa data
 */
void ldns_key_assign_rsa_key(ldns_key *k, RSA *r);

/**
 * Assign the key's dsa data
 * The dsa data will be freed automatically when the key is freed.
 * \param[in] k the key
 * \param[in] d the dsa data
 */
void ldns_key_assign_dsa_key(ldns_key *k, DSA *d);

/** 
 * Get the PKEY id for GOST, loads GOST into openssl as a side effect.
 * Only available if GOST is compiled into the library and openssl.
 * \return the gost id for EVP_CTX creation.
 */
int ldns_key_EVP_load_gost_id(void);

/** Release the engine reference held for the GOST engine. */
void ldns_key_EVP_unload_gost(void);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

/**
 * Set the key's hmac data
 * \param[in] k the key
 * \param[in] hmac the raw key data
 */
void ldns_key_set_hmac_key(ldns_key *k, unsigned char *hmac);

/**
 * Set the key id data. This is used if the key points to
 * some externally stored key data
 * 
 * Only the pointer is set, the data there is not copied,
 * and must be freed manually; ldns_key_deep_free() does 
 * *not* free this data
 * \param[in] key the key
 * \param[in] external_key key id data
 */
void ldns_key_set_external_key(ldns_key *key, void *external_key);

/**
 * Set the key's hmac size
 * \param[in] k the key
 * \param[in] hmac_size the size of the hmac data
 */
void ldns_key_set_hmac_size(ldns_key *k, size_t hmac_size);
/**
 * Set the key's original ttl
 * \param[in] k the key
 * \param[in] t the ttl
 */
void ldns_key_set_origttl(ldns_key *k, uint32_t t);
/**
 * Set the key's inception date (seconds after epoch)
 * \param[in] k the key
 * \param[in] i the inception
 */
void ldns_key_set_inception(ldns_key *k, uint32_t i);
/**
 * Set the key's expiration date (seconds after epoch)
 * \param[in] k the key
 * \param[in] e the expiration
 */
void ldns_key_set_expiration(ldns_key *k, uint32_t e);
/**
 * Set the key's pubkey owner
 * \param[in] k the key
 * \param[in] r the owner
 */
void ldns_key_set_pubkey_owner(ldns_key *k, ldns_rdf *r);
/**
 * Set the key's key tag
 * \param[in] k the key
 * \param[in] tag the keytag
 */
void ldns_key_set_keytag(ldns_key *k, uint16_t tag);
/**
 * Set the key's flags
 * \param[in] k the key
 * \param[in] flags the flags
 */
void ldns_key_set_flags(ldns_key *k, uint16_t flags);
/**
 * Set the keylist's key count to count
 * \param[in] key the key
 * \param[in] count the cuont
 */
void ldns_key_list_set_key_count(ldns_key_list *key, size_t count);

/**     
 * pushes a key to a keylist
 * \param[in] key_list the key_list to push to 
 * \param[in] key the key to push 
 * \return false on error, otherwise true
 */      
bool ldns_key_list_push_key(ldns_key_list *key_list, ldns_key *key);

/**
 * returns the number of keys in the key list
 * \param[in] key_list the key_list
 * \return the numbers of keys in the list
 */
size_t ldns_key_list_key_count(const ldns_key_list *key_list);

/**
 * returns a pointer to the key in the list at the given position
 * \param[in] key the key
 * \param[in] nr the position in the list
 * \return the key
 */
ldns_key *ldns_key_list_key(const ldns_key_list *key, size_t nr);

#if LDNS_BUILD_CONFIG_HAVE_SSL
/**
 * returns the (openssl) RSA struct contained in the key
 * \param[in] k the key to look in
 * \return the RSA * structure in the key
 */
RSA *ldns_key_rsa_key(const ldns_key *k);
/**
 * returns the (openssl) EVP struct contained in the key
 * \param[in] k the key to look in
 * \return the RSA * structure in the key
 */
EVP_PKEY *ldns_key_evp_key(const ldns_key *k);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

/**
 * returns the (openssl) DSA struct contained in the key
 */
#if LDNS_BUILD_CONFIG_HAVE_SSL
DSA *ldns_key_dsa_key(const ldns_key *k);
#endif /* LDNS_BUILD_CONFIG_HAVE_SSL */

/**
 * return the signing alg of the key
 * \param[in] k the key
 * \return the algorithm
 */
ldns_signing_algorithm ldns_key_algorithm(const ldns_key *k);
/**
 * set the use flag
 * \param[in] k the key
 * \param[in] v the boolean value to set the _use field to
 */
void ldns_key_set_use(ldns_key *k, bool v);
/**
 * return the use flag
 * \param[in] k the key
 * \return the boolean value of the _use field
 */
bool ldns_key_use(const ldns_key *k);
/**
 * return the hmac key data
 * \param[in] k the key
 * \return the hmac key data
 */
unsigned char *ldns_key_hmac_key(const ldns_key *k);
/**
 * return the key id key data
 * \param[in] k the key
 * \return the key id data
 */
void *ldns_key_external_key(const ldns_key *k);
/**
 * return the hmac key size
 * \param[in] k the key
 * \return the hmac key size
 */
size_t ldns_key_hmac_size(const ldns_key *k);
/**
 * return the original ttl of the key
 * \param[in] k the key
 * \return the original ttl
 */
uint32_t ldns_key_origttl(const ldns_key *k);
/**
 * return the key's inception date
 * \param[in] k the key
 * \return the inception date
 */
uint32_t ldns_key_inception(const ldns_key *k);
/**
 * return the key's expiration date
 * \param[in] k the key
 * \return the experiration date
 */
uint32_t ldns_key_expiration(const ldns_key *k);
/**
 * return the keytag
 * \param[in] k the key
 * \return the keytag
 */
uint16_t ldns_key_keytag(const ldns_key *k);
/**
 * return the public key's owner
 * \param[in] k the key
 * \return the owner
 */
ldns_rdf *ldns_key_pubkey_owner(const ldns_key *k);
/**
 * Set the 'use' flag for all keys in the list
 * \param[in] keys The key_list
 * \param[in] v The value to set the use flags to
 */
void
ldns_key_list_set_use(ldns_key_list *keys, bool v);

/**
 * return the flag of the key
 * \param[in] k the key
 * \return the flag
 */
uint16_t ldns_key_flags(const ldns_key *k);

/**     
 * pops the last rr from a keylist
 * \param[in] key_list the rr_list to pop from
 * \return NULL if nothing to pop. Otherwise the popped RR
 */
ldns_key *ldns_key_list_pop_key(ldns_key_list *key_list);

/** 
 * converts a ldns_key to a public key rr
 * If the key data exists at an external point, the corresponding
 * rdata field must still be added with ldns_rr_rdf_push() to the
 * result rr of this function
 *
 * \param[in] k the ldns_key to convert
 * \return ldns_rr representation of the key
 */
ldns_rr *ldns_key2rr(const ldns_key *k);

/**
 * print a private key to the file ouput
 * 
 * \param[in] output the FILE descriptor where to print to
 * \param[in] k the ldns_key to print
 */
void ldns_key_print(FILE *output, const ldns_key *k);

/**
 * frees a key structure, but not its internal data structures
 *
 * \param[in] key the key object to free
 */
void ldns_key_free(ldns_key *key);

/**
 * frees a key structure and all its internal data structures, except
 * the data set by ldns_key_set_external_key()
 *
 * \param[in] key the key object to free
 */
void ldns_key_deep_free(ldns_key *key);

/**
 * Frees a key list structure
 * \param[in] key_list the key list object to free
 */
void ldns_key_list_free(ldns_key_list *key_list);

/**
 * Instantiates a DNSKEY or DS RR from file.
 * \param[in] filename the file to read the record from
 * \return the corresponding RR, or NULL if the parsing failed
 */
ldns_rr * ldns_read_anchor_file(const char *filename);

/**
 * Returns the 'default base name' for key files;
 * IE. K\<zone\>+\<alg\>+\<keytag\>
 * (without the .key or .private)
 * The memory for this is allocated by this function,
 * and should be freed by the caller
 * 
 * \param[in] key the key to get the file name from
 * \returns A string containing the file base name
 */
char *ldns_key_get_file_base_name(ldns_key *key);

/**
 * See if a key algorithm is supported
 * \param[in] algo the signing algorithm number.
 * \returns true if supported.
 */
int ldns_key_algo_supported(int algo);

/**
 * Get signing algorithm by name.  Comparison is case insensitive.
 * \param[in] name string with the name.
 * \returns 0 on parse failure or the algorithm number.
 */
ldns_signing_algorithm ldns_get_signing_algorithm_by_name(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* LDNS_KEYS_H */
