/*
 * Copyright (c) 1995-1998 John Birrell <jb@cimlogic.com.au>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by John Birrell.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
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
 */
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/signalvar.h>
#include "thr_private.h"

__weak_reference(_fork, fork);

pid_t
_fork(void)
{
	sigset_t sigset, oldset;
	struct pthread *curthread;
	pid_t ret;

	if (!_kse_isthreaded())
		return (__sys_fork());

	curthread = _get_curthread();

	/*
	 * Masks all signals until we reach a safe point in
	 * _kse_single_thread, and the signal masks will be
	 * restored in that function, for M:N thread, all 
	 * signals were already masked in kernel atomically,
	 * we only need to do this for bound thread.
	 */
	if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM) {
		SIGFILLSET(sigset);
		__sys_sigprocmask(SIG_SETMASK, &sigset, &oldset);
	}
	/* Fork a new process: */
	if ((ret = __sys_fork()) == 0) {
		/* Child process */
		_kse_single_thread(curthread);
		/* Kernel signal mask is restored in _kse_single_thread */
	} else {
		if (curthread->attr.flags & PTHREAD_SCOPE_SYSTEM)
			__sys_sigprocmask(SIG_SETMASK, &oldset, NULL);
	}

	/* Return the process ID: */
	return (ret);
}
