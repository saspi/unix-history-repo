/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)types.h	7.6 (Berkeley) %G%
 */

#ifndef	_MACHTYPES_H_
#define	_MACHTYPES_H_

typedef struct _physadr {
	int r[1];
} *physadr;

typedef struct label_t {
	int val[14];
} label_t;

#ifdef	__GNUC__
typedef	char			s1byte_t;	/* Basic data types. */
typedef	unsigned char		u1byte_t;
typedef	short			s2byte_t;
typedef	unsigned short		u2byte_t;
typedef	long			s4byte_t;
typedef	unsigned long		u4byte_t;
typedef	long long		s8byte_t;
typedef	unsigned long long	u8byte_t;
typedef	float			f4byte_t;
typedef	double			f8byte_t;
#endif

#endif	/* _MACHTYPES_H_ */
