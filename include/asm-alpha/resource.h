#ifndef _ALPHA_RESOURCE_H
#define _ALPHA_RESOURCE_H

/*
 * Resource limits
 */

#define RLIMIT_CPU	0		/* CPU time in ms */
#define RLIMIT_FSIZE	1		/* Maximum filesize */
#define RLIMIT_DATA	2		/* max data size */
#define RLIMIT_STACK	3		/* max stack size */
#define RLIMIT_CORE	4		/* max core file size */
#define RLIMIT_RSS	5		/* max resident set size */
#define RLIMIT_NOFILE	6		/* max number of open files */
#define RLIMIT_AS	7		/* address space limit(?) */
#define RLIMIT_NPROC	8		/* max number of processes */
#define RLIMIT_MEMLOCK	9		/* max locked-in-memory address space */
#define RLIMIT_LOCKS	10		/* maximum file locks held */
#define RLIMIT_SIGPENDING 11		/* max number of pending signals */
#define RLIMIT_MSGQUEUE 12		/* maximum bytes in POSIX mqueues */

#define RLIM_NLIMITS	13

/*
 * SuS says limits have to be unsigned.  Fine, it's unsigned, but
 * we retain the old value for compatibility, especially with DU. 
 * When you run into the 2^63 barrier, you call me.
 */
#define RLIM_INFINITY	0x7ffffffffffffffful

#ifdef __KERNEL__

#define INIT_RLIMITS							\
{									\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_CPU */	\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_FSIZE */	\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_DATA */	\
    {_STK_LIM, LONG_MAX},			/* RLIMIT_STACK */	\
    {       0, LONG_MAX},			/* RLIMIT_CORE */	\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_RSS */	\
    {INR_OPEN, INR_OPEN},			/* RLIMIT_NOFILE */	\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_AS */		\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_NPROC */	\
    {MLOCK_LIMIT, MLOCK_LIMIT },		/* RLIMIT_MEMLOCK */	\
    {LONG_MAX, LONG_MAX},			/* RLIMIT_LOCKS */	\
    {MAX_SIGPENDING, MAX_SIGPENDING},		/* RLIMIT_SIGPENDING */ \
    {MQ_BYTES_MAX, MQ_BYTES_MAX},		/* RLIMIT_MSGQUEUE */	\
}

#endif /* __KERNEL__ */

#endif /* _ALPHA_RESOURCE_H */
