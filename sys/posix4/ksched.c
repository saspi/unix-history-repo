/*
 * Copyright (c) 1996, 1997
 *	HD Associates, Inc.  All rights reserved.
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
 *	This product includes software developed by HD Associates, Inc
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY HD ASSOCIATES AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL HD ASSOCIATES OR CONTRIBUTORS BE LIABLE
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

/* ksched: Soft real time scheduling based on "rtprio".
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/resource.h>
#include <sys/sched.h>

#include <posix4/posix4.h>

/* ksched: Real-time extension to support POSIX priority scheduling.
 */

struct ksched {
	struct timespec rr_interval;
};

int ksched_attach(struct ksched **p)
{
	struct ksched *ksched= p31b_malloc(sizeof(*ksched));

	ksched->rr_interval.tv_sec = 0;
	ksched->rr_interval.tv_nsec = 1000000000L / sched_rr_interval();

	*p = ksched;
	return 0;
}

int ksched_detach(struct ksched *ks)
{
	p31b_free(ks);

	return 0;
}

/*
 * XXX About priorities
 *
 *	POSIX 1003.1b requires that numerically higher priorities be of
 *	higher priority.  It also permits sched_setparam to be
 *	implementation defined for SCHED_OTHER.  I don't like
 *	the notion of inverted priorites for normal processes when
 *  you can use "setpriority" for that.
 *
 *	I'm rejecting sched_setparam for SCHED_OTHER with EINVAL.
 */

/* Macros to convert between the unix (lower numerically is higher priority)
 * and POSIX 1003.1b (higher numerically is higher priority)
 */

#define p4prio_to_rtpprio(P) (RTP_PRIO_MAX - (P))
#define rtpprio_to_p4prio(P) (RTP_PRIO_MAX - (P))

/* These improve readability a bit for me:
 */
#define P1B_PRIO_MIN rtpprio_to_p4prio(RTP_PRIO_MAX)
#define P1B_PRIO_MAX rtpprio_to_p4prio(RTP_PRIO_MIN)

static __inline int
getscheduler(register_t *ret, struct ksched *ksched, struct thread *td)
{
	struct rtprio rtp;
	int e = 0;

	mtx_lock_spin(&sched_lock);
	pri_to_rtp(td->td_ksegrp, &rtp);
	mtx_unlock_spin(&sched_lock);
	switch (rtp.type)
	{
		case RTP_PRIO_FIFO:
		*ret = SCHED_FIFO;
		break;

		case RTP_PRIO_REALTIME:
		*ret = SCHED_RR;
		break;

		default:
		*ret = SCHED_OTHER;
		break;
	}

	return e;
}

int ksched_setparam(register_t *ret, struct ksched *ksched,
	struct thread *td, const struct sched_param *param)
{
	register_t policy;
	int e;

	e = getscheduler(&policy, ksched, td);

	if (e == 0)
	{
		if (policy == SCHED_OTHER)
			e = EINVAL;
		else
			e = ksched_setscheduler(ret, ksched, td, policy, param);
	}

	return e;
}

int ksched_getparam(register_t *ret, struct ksched *ksched,
	struct thread *td, struct sched_param *param)
{
	struct rtprio rtp;

	mtx_lock_spin(&sched_lock);
	pri_to_rtp(td->td_ksegrp, &rtp);
	mtx_unlock_spin(&sched_lock);
	if (RTP_PRIO_IS_REALTIME(rtp.type))
		param->sched_priority = rtpprio_to_p4prio(rtp.prio);

	return 0;
}

/*
 * XXX The priority and scheduler modifications should
 *     be moved into published interfaces in kern/kern_sync.
 *
 * The permissions to modify process p were checked in "p31b_proc()".
 *
 */
int ksched_setscheduler(register_t *ret, struct ksched *ksched,
	struct thread *td, int policy, const struct sched_param *param)
{
	int e = 0;
	struct rtprio rtp;
	struct ksegrp *kg = td->td_ksegrp;

	switch(policy)
	{
		case SCHED_RR:
		case SCHED_FIFO:

		if (param->sched_priority >= P1B_PRIO_MIN &&
		param->sched_priority <= P1B_PRIO_MAX)
		{
			rtp.prio = p4prio_to_rtpprio(param->sched_priority);
			rtp.type = (policy == SCHED_FIFO)
				? RTP_PRIO_FIFO : RTP_PRIO_REALTIME;

			mtx_lock_spin(&sched_lock);
			rtp_to_pri(&rtp, kg);
			FOREACH_THREAD_IN_GROUP(kg, td) { /* XXXKSE */
				if (TD_IS_RUNNING(td)) {
					td->td_kse->ke_flags |= KEF_NEEDRESCHED;
				} else if (TD_ON_RUNQ(td)) {
					if (td->td_priority > kg->kg_user_pri) {
						remrunqueue(td);
						td->td_priority =
						    kg->kg_user_pri;
						setrunqueue(td);
					}
				}
			}
			mtx_unlock_spin(&sched_lock);
		}
		else
			e = EPERM;


		break;

		case SCHED_OTHER:
		{
			rtp.type = RTP_PRIO_NORMAL;
			rtp.prio = p4prio_to_rtpprio(param->sched_priority);
			mtx_lock_spin(&sched_lock);
			rtp_to_pri(&rtp, kg);

			/* XXX Simply revert to whatever we had for last
			 *     normal scheduler priorities.
			 *     This puts a requirement
			 *     on the scheduling code: You must leave the
			 *     scheduling info alone.
			 */
			FOREACH_THREAD_IN_GROUP(kg, td) {
				if (TD_IS_RUNNING(td)) {
					td->td_kse->ke_flags |= KEF_NEEDRESCHED;
				} else if (TD_ON_RUNQ(td)) {
					if (td->td_priority > kg->kg_user_pri) {
						remrunqueue(td);
						td->td_priority =
						    kg->kg_user_pri;
						setrunqueue(td);
					}
				}
				
			}
			mtx_unlock_spin(&sched_lock);
		}
		break;
	}

	return e;
}

int ksched_getscheduler(register_t *ret, struct ksched *ksched, struct thread *td)
{
	return getscheduler(ret, ksched, td);
}

/* ksched_yield: Yield the CPU.
 */
int ksched_yield(register_t *ret, struct ksched *ksched)
{
	mtx_lock_spin(&sched_lock);
	curthread->td_kse->ke_flags |= KEF_NEEDRESCHED;
	mtx_unlock_spin(&sched_lock);
	return 0;
}

int ksched_get_priority_max(register_t*ret, struct ksched *ksched, int policy)
{
	int e = 0;

	switch (policy)
	{
		case SCHED_FIFO:
		case SCHED_RR:
		*ret = RTP_PRIO_MAX;
		break;

		case SCHED_OTHER:
		*ret =  PRIO_MAX;
		break;

		default:
		e = EINVAL;
	}

	return e;
}

int ksched_get_priority_min(register_t *ret, struct ksched *ksched, int policy)
{
	int e = 0;

	switch (policy)
	{
		case SCHED_FIFO:
		case SCHED_RR:
		*ret = P1B_PRIO_MIN;
		break;

		case SCHED_OTHER:
		*ret =  PRIO_MIN;
		break;

		default:
		e = EINVAL;
	}

	return e;
}

int ksched_rr_get_interval(register_t *ret, struct ksched *ksched,
	struct thread *td, struct timespec *timespec)
{
	*timespec = ksched->rr_interval;

	return 0;
}
