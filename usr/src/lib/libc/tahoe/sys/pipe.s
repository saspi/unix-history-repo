/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
_sccsid:.asciz	"@(#)pipe.s	5.2 (Berkeley) %G%"
#endif /* SYSLIBC_SCCS and not lint */

#include "SYS.h"

SYSCALL(pipe)
	movl	4(fp),r2
	movl	r0,(r2)
	movl	r1,4(r2)
	clrl	r0
	ret
