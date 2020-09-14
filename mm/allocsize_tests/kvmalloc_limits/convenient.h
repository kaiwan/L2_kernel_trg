/*
 * convenient.h
 *
 * A few convenience routines..
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * GPL / LGPL
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __CONVENIENT_H__
#define __CONVENIENT_H__

#include <asm/param.h>	/* HZ */
#include <linux/sched.h>


/*------------------------ MSG, QP ------------------------------------*/

#ifdef __KERNEL__
#include <linux/interrupt.h>
#define PRINT_CTX() {        \
  if (printk_ratelimit()) { \
	  printk("PRINT_CTX:: in function %s on cpu #%2d\n", __func__, smp_processor_id()); \
      if (!in_interrupt()) \
	  	printk(" in process context: %s:%d\n", current->comm, current->pid); \
	  else \
        printk(" in interrupt context: in_interrupt:%3s. in_irq:%3s. in_softirq:%3s. in_serving_softirq:%3s. preempt_count=0x%x\n",  \
          (in_interrupt()?"yes":"no"), (in_irq()?"yes":"no"), (in_softirq()?"yes":"no"),        \
          (in_serving_softirq()?"yes":"no"), preempt_count());        \
  } \
}
#endif

#ifdef DEBUG
    #ifdef __KERNEL__
  	  #define MSG(string, args...) \
		 printk(KERN_INFO "%s:%d : " string, __FUNCTION__, __LINE__, ##args)
	#else
  	#define MSG(string, args...) \
		fprintf(stderr, "%s:%d : " string, __FUNCTION__, __LINE__, ##args)
	#endif

    #ifdef __KERNEL__
  	#define MSG_SHORT(string, args...) \
			printk(KERN_INFO string, ##args)
	#else
  	#define MSG_SHORT(string, args...) \
			fprintf(stderr, string, ##args)
	#endif
    
	#define QP MSG("\n")
    
    #ifdef __KERNEL__
	#define QPDS do { \
		 MSG("\n"); \
		 dump_stack(); \
	  } while(0)
	#define PRCS_CTX do { \
		 if (!in_interrupt()) { \
			MSG("prcs ctx: %s(%d)\n", current->comm, current->pid); \
		 } \
		 else { \
			MSG("irq ctx\n"); \
			PRINT_IRQCTX();   \
		} \
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



/*------------------------ assert ---------------------------------------*/
#ifdef __KERNEL__
#define assert(expr) \
if (!(expr)) { \
 printk("********** Assertion [%s] failed! : %s:%s:%d **********\n", \
  #expr, __FILE__, __func__, __LINE__); \
}
#endif

/*------------------------ DELAY_LOOP --------------------------------*/
static inline void beep(int what)
{
  #ifdef __KERNEL__
	(void)printk(KERN_INFO "%c", (char)what );
  #else
	(void)printf("%c", (char)what );
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
static int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
/* Subtract the `struct timeval' values X and Y,
    storing the result in RESULT.
    Return 1 if the difference is negative, otherwise 0.  */
__attribute__ ((unused)) 
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
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
  int  k = 0, n = 0;
  int  neg_flag = 0;
  int  remain;
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
  int  old_decimal;  // for test
#endif
  char temp[80];
 
  // take care of negative input
  if (decimal < 0)
  {      
    decimal = -decimal;
    neg_flag = 1;
  }
  do 
  {
#ifdef TESTMODE
    old_decimal = decimal;   // for test
#endif
    remain    = decimal % 2;
    // whittle down the decimal number
    decimal   = decimal / 2;
    // this is a test to show the action
#ifdef TESTMODE
    printf("%d/2 = %d  remainder = %d\n", old_decimal, decimal, remain);
#endif
    // converts digit 0 or 1 to character '0' or '1'
    temp[k++] = remain + '0';
  } while (decimal > 0);
 
  if (neg_flag)
    temp[k++] = '-';       // add - sign
  else
    temp[k++] = ' ';       // space
 
  // reverse the spelling
  while (k >= 0)
    binary[n++] = temp[--k];
 
  binary[n-1] = 0;         // end with NULL
}


#ifndef __KERNEL__

/* Verbose printf ... */
#define VP(verbose, str, args...) \
do { \
	if (verbose) \
		printf(str, ##args); \
} while (0)

static inline int err_exit(char *prg, char * err, int exitcode)
{
	char err_str[512];

	snprintf(err_str, 511, "%s: %s", prg, err);
	perror(err_str);
	exit(exitcode);
} // err_exit()

#endif

#endif
