/* mdiv -- BSD compatible divide producing both remainder and quotient.

Copyright (C) 1991, 1994, 1996 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include "mp.h"
#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

void
#if __STDC__
mdiv (const MINT *num, const MINT *den, MINT *quot, MINT *rem)
#else
mdiv (num, den, quot, rem)
     const MINT *num;
     const MINT *den;
     MINT *quot;
     MINT *rem;
#endif

#define COMPUTE_QUOTIENT
#include "dmincl.c"
