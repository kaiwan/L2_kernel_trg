/*
 * convenient.h
 *
 * A few convenience macros and routines..
 * Mostly for kernel-space usage, some for user-space as well.
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * (c) Kaiwan NB, kaiwanTECH
 * GPL / LGPL
 *
 */
#ifndef __CONVENIENT_H__
#define __CONVENIENT_H__

#include <asm/param.h>		/* HZ */
#include <linux/sched.h>

#ifdef __KERNEL__
#include <linux/ratelimit.h>

/* 
   *** Note: PLEASE READ this documentation: ***

    We can reduce the load, and increase readability, by using the trace_printk
    instead of printk. To see the o/p do:
     # cat /sys/kernel/debug/tracing/trace

     If we insist on using the regular printk, lets at least rate-limit it.
	 For the programmers' convenience, this too is programatically controlled 
	  (by an integer var USE_RATELIMITING [default: On]).

 	*** Kernel module authors Note: ***
	To use the trace_printk(), pl #define the symbol USE_FTRACE_PRINT in your Makefile:
	EXTRA_CFLAGS += -DUSE_FTRACE_PRINT
	If you do not do this, we will use the usual printk() .

	To view :
	  printk's       : dmesg
      trace_printk's : cat /sys/kernel/debug/tracing/trace

	 Default: printk
 */
// keep this defined to use the FTRACE-style trace_printk(), else will use regular printk()
//#define USE_FTRACE_PRINT
#undef USE_FTRACE_PRINT

#ifdef USE_FTRACE_PRINT
 #define DBGPRINT(string, args...) \
     trace_printk(string, ##args);
#else
 #define DBGPRINT(string, args...) do {                 \
     int USE_RATELIMITING=1;                            \
	 /* Not supposed to use printk_ratelimit() now.. */ \
	 if (USE_RATELIMITING) {                            \
        printk_ratelimited (KERN_INFO pr_fmt(string), ##args);  \
	 }                                                  \
	 else                                               \
        printk (KERN_INFO pr_fmt(string), ##args);              \
 } while (0)
#endif

/*------------------------ MSG, QP ------------------------------------*/
#ifdef DEBUG
#ifdef __KERNEL__
#define MSG(string, args...) do {                                     \
		 DBGPRINT("%s:%d : " string, __FUNCTION__, __LINE__, ##args);      \
	 } while (0)
#else
#define MSG(string, args...) do {                                      \
		fprintf(stderr, "%s:%d : " string, __FUNCTION__, __LINE__, ##args); \
	 } while (0)
#endif

#ifdef __KERNEL__
#define MSG_SHORT(string, args...) do { \
			DBGPRINT(string, ##args);        \
	 } while (0)
#else
#define MSG_SHORT(string, args...) do { \
			fprintf(stderr, string, ##args); \
	 } while (0)
#endif

// QP = Quick Print
#define QP MSG("\n")

#ifdef __KERNEL__
#define QPDS do { \
		 MSG("\n");    \
		 dump_stack(); \
	  } while(0)
#endif

#ifdef __KERNEL__
#define HexDump(from_addr, len) \
 	    print_hex_dump_bytes (" ", DUMP_PREFIX_ADDRESS, from_addr, len);
#endif
#else
#define MSG(string, args...)
#define MSG_SHORT(string, args...)
#define QP
#define QPDS
#endif

/*------------------------ PRINT_CTX ---------------------------------*/
/* 
 An interesting way to print the context info:
 If USE_FTRACE_PRINT is On, it implies we'll use trace_printk(), else the vanilla
 printk(). 
 If we are using trace_printk(), we will automatically get output in the ftrace 
 latency format (see below):

 * The Ftrace 'latency-format' :
                       _-----=> irqs-off          [d]
                      / _----=> need-resched      [N]
                     | / _---=> hardirq/softirq   [H|h|s]   H=>both h && s
                     || / _--=> preempt-depth     [#]
                     ||| /                      
 CPU  TASK/PID       ||||  DURATION                  FUNCTION CALLS 
 |     |    |        ||||   |   |                     |   |   |   | 

 However, if we're _not_ using ftrace trace_printk(), then we'll _emulate_ the same
 with the printk() !
 (Of course, without the 'Duration' and 'Function Calls' fields).
 */
#include <linux/sched.h>
#include <linux/interrupt.h>

#ifndef USE_FTRACE_PRINT	// 'normal' printk(), lets emulate ftrace latency format
#define PRINT_CTX() do {                                                                     \
	char sep='|', intr='.';                                                              \
	                                                                                     \
   if (in_interrupt()) {                                                                     \
      if (in_irq() && in_softirq())                                                          \
	    intr='H';                                                                        \
	  else if (in_irq())                                                                 \
	    intr='h';                                                                        \
	  else if (in_softirq())                                                             \
	    intr='s';                                                                        \
	}                                                                                    \
   else                                                                                      \
	intr='.';                                                                            \
	                                                                                     \
	DBGPRINT(                                                                            \
	"PRINT_CTX:: [%03d]%c%s%c:%d   %c "                                                  \
	"%c%c%c%u "                                                                          \
	"\n"                                                                                 \
	, smp_processor_id(),                                                                \
    (!current->mm?'[':' '), current->comm, (!current->mm?']':' '), current->pid, sep,        \
	(irqs_disabled()?'d':'.'),                                                           \
	(need_resched()?'N':'.'),                                                            \
	intr,                                                                                \
	(preempt_count() && 0xff)                                                            \
	);                                                                                   \
} while (0)
#else				// using ftrace trace_prink() internally
#define PRINT_CTX() do {                                                                   \
	DBGPRINT("PRINT_CTX:: [cpu %02d]%s:%d\n", smp_processor_id(), __func__, current->pid); \
	if (!in_interrupt()) {                                                                 \
  		DBGPRINT(" in process context:%c%s%c:%d\n",                                        \
		    (!current->mm?'[':' '), current->comm, (!current->mm?']':' '), current->pid);  \
	} else {                                                                               \
        DBGPRINT(" in interrupt context: in_interrupt:%3s. in_irq:%3s. in_softirq:%3s. "   \
		"in_serving_softirq:%3s. preempt_count=0x%x\n",                                    \
          (in_interrupt()?"yes":"no"), (in_irq()?"yes":"no"), (in_softirq()?"yes":"no"),   \
          (in_serving_softirq()?"yes":"no"), (preempt_count() && 0xff));                   \
	}                                                                                      \
} while (0)
#endif
#endif				// end ifdef __KERNEL__ at the top

/*------------------------ assert ---------------------------------------
 * Hey you, careful! 
 * Using assertions is great *but* pl be aware of traps & pitfalls:
 * http://blog.regehr.org/archives/1096
 *
 * The closest equivalent perhaps, to assert() in the kernel are the BUG() 
 * or BUG_ON() and WARN() or WARN_ON() macros. Using BUG*() is _only_ for those
 * cases where recovery is impossible. WARN*() is usally considered a better
 * option. Pl see <asm-generic/bug.h> for details.
 *
 * Here, we just trivially emit a noisy [trace_]printk() to "warn" the dev/user.
 */
#ifdef __KERNEL__
#define assert(expr) do {                                               \
 if (!(expr)) {                                                         \
  DBGPRINT("********** Assertion [%s] failed! : %s:%s:%d **********\n", \
   #expr, __FILE__, __func__, __LINE__);                                \
 }                                                                      \
} while(0)
#endif

/*------------------------ DELAY_LOOP --------------------------------*/
static inline void beep(int what)
{
#ifdef __KERNEL__
	DBGPRINT("%c", (char)what);
#else
	(void)printf("%c", (char)what);
#endif
}

/* 
 * DELAY_LOOP macro
 * @val : ASCII value to print
 * @loop_count : times to loop around
 */
#define DELAY_LOOP(val,loop_count) \
{ \
	int c=0, m;\
	unsigned int for_index,inner_index; \
	\
	for(for_index=0;for_index<loop_count;for_index++) { \
		beep((val)); \
		c++;\
			for(inner_index=0;inner_index<HZ*1000*8;inner_index++) \
				for(m=0;m<50;m++); \
		} \
	/*printf("c=%d\n",c);*/\
}
/*------------------------------------------------------------------------*/

#ifdef __KERNEL__
/*------------ DELAY_SEC -------------------------*
 * Delays execution for n seconds.
 * MUST be called from process context.
 *------------------------------------------------*/
#define DELAY_SEC(val) \
{ \
	if (!in_interrupt()) {	\
		set_current_state (TASK_INTERRUPTIBLE); \
		schedule_timeout (val * HZ); \
	}	\
}
#endif

/* Get time difference between two struct timeval's
 * Credits: Arkaitz Jimenez
 * http://stackoverflow.com/questions/1444428/time-stamp-in-the-c-programming-language
 */
static int timeval_subtract(struct timeval *result, struct timeval *x,
			    struct timeval *y);
/* Subtract the `struct timeval' values X and Y,
    storing the result in RESULT.
    Return 1 if the difference is negative, otherwise 0.  */
__attribute__ ((unused))
int timeval_subtract(struct timeval *result, struct timeval *x,
		     struct timeval *y)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	   tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

/*
 * Converts decimal to binary. 
 * Credits: vegaseat. URL: http://www.daniweb.com/software-development/c/code/216349
 * accepts a decimal integer and returns a binary coded string
 *
 * @decimal : decimal value to convert to binary (IN)
 * @binary  : the binary result as a string (OUT)
 *
 */
__attribute__ ((unused))
static void dec2bin(long decimal, char *binary)
{
	int k = 0, n = 0;
	int neg_flag = 0;
	int remain;
	/*
	   gcc 4.6.3 : we get the warning:
	   "warning: variable ‘old_decimal’ set but not used [-Wunused-but-set-variable]"
	   To get rid of this warning, have #ifdef'd the test... -kaiwan.

	   Keep one of the following below (wrt TESTMODE); comment out the other.
	   UN-defining by default.
	 */
//#define TESTMODE
#undef TESTMODE

#ifdef TESTMODE
	int old_decimal;	// for test
#endif
	char temp[80];

	// take care of negative input
	if (decimal < 0) {
		decimal = -decimal;
		neg_flag = 1;
	}
	do {
#ifdef TESTMODE
		old_decimal = decimal;	// for test
#endif
		remain = decimal % 2;
		// whittle down the decimal number
		decimal = decimal / 2;
		// this is a test to show the action
#ifdef TESTMODE
		printf("%d/2 = %d  remainder = %d\n", old_decimal, decimal,
		       remain);
#endif
		// converts digit 0 or 1 to character '0' or '1'
		temp[k++] = remain + '0';
	} while (decimal > 0);

	if (neg_flag)
		temp[k++] = '-';	// add - sign
	else
		temp[k++] = ' ';	// space

	// reverse the spelling
	while (k >= 0)
		binary[n++] = temp[--k];

	binary[n - 1] = 0;	// end with NULL
}

#ifndef __KERNEL__

#define MSG(string, args...) do {                                      \
	fprintf(stderr, "%s:%d : " string, __FUNCTION__, __LINE__, ##args); \
} while (0)

#define MSG_SHORT(string, args...) do { \
		fprintf(stderr, string, ##args); \
} while (0)

#define QP MSG("\n")

/* Verbose printf ... */
#define VP(verbose, str, args...) \
do { \
	if (verbose) \
		printf(str, ##args); \
} while (0)

static inline int err_exit(char *prg, char *err, int exitcode)
{
	char err_str[512];

	snprintf(err_str, 511, "%s: %s", prg, err);
	perror(err_str);
	exit(exitcode);
}				// err_exit()

#endif

#endif
