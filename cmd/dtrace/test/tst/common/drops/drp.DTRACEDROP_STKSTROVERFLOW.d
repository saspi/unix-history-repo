/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Copyright (c) 2012 by Delphix. All rights reserved.
 */

#pragma D option destructive
#pragma D option jstackstrsize=1
#pragma D option quiet

BEGIN
{
	/*
	 * Since some java versions are erroneously compiled with -xlazyload
	 * and therefore don't activate their ustack() helper by default, we
	 * use the DTrace helper audit library to activate it.
	 */
	system("LD_AUDIT_32=/usr/lib/dtrace/libdtrace_forceload.so %s",
	    "java -version");
}

syscall:::entry
{
	@[jstack()] = count();
}

proc:::exit
/progenyof($pid) && execname == "java"/
{
	exit(0);
}

END
{
	printa("\r", @);
	printf("\n");
}
