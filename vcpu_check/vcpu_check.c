/*
 * ch5/timer_simple/timer_simple.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming - Part 2"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2
 *
 * From: Ch 5 : Timers, kernel threads and more
 ****************************************************************
 * Brief Description:
 * A demo of a simple kernel timer in action. We make use of the from_timer()
 * to be able to access data from within the callback, and setup the timer to
 * keep expiring until our 'data' variable hits zero.
 *
 * For details, please refer the book, Ch 5.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/sched/signal.h>
#include "../convenient.h"

#define INITIAL_VALUE	120

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("A simple LKM to demo a (repeating) kernel timer");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static struct st_ctx {
	struct timer_list tmr;
	int data;
} ctx;
static unsigned long exp_ms = 10;

/*
 * ding() - our timer's callback function!
 */
static void ding(struct timer_list *timer)
{
	struct st_ctx *priv = from_timer(priv, timer, tmr);
	/* from_timer() is in fact a wrapper around the well known
	 * container_of() macro! This allows us to retrieve access to our
	 * 'parent' driver context structure
	 */
	struct task_struct *p, *t;
	//u64 t1, t2;

	// Whoops! Bugfix- decrement even if DEBUG is off...
	//priv->data--;
	//pr_debug("timed out... data=%d\n", priv->data);

	//t1 = ktime_get_real_ns();
	for_each_process_thread(p, t) {
		//PRINT_CTX(t);
		if (t->flags & PF_VCPU) {
			PRINT_CTX(t);
			pr_info("*** PF_VCPU set ***\n");
		}
		/* else
			pr_info("%s:%d: PF_VCPU unset\n", t->comm, t->pid);
		 */
	}
	//t2 = ktime_get_real_ns();
	//SHOW_DELTA(t2, t1);

	/* until countdown done, fire it again! */
	//if (priv->data)
	mod_timer(&priv->tmr, jiffies + msecs_to_jiffies(exp_ms));
}

static int __init timer_simple_init(void)
{
	ctx.data = INITIAL_VALUE;

	/* Initialize our kernel timer */
	ctx.tmr.expires = jiffies + msecs_to_jiffies(exp_ms);
	ctx.tmr.flags = 0;
	timer_setup(&ctx.tmr, ding, 0);

	pr_info("timer set to expire in %ld ms\n", exp_ms);
	add_timer(&ctx.tmr); /* Arm it; lets get going! */

	return 0;		/* success */
}

static void __exit timer_simple_exit(void)
{
	// Wait for possible timeouts to complete... and then delete the timer
	del_timer_sync(&ctx.tmr);
	pr_info("removed\n");
}

module_init(timer_simple_init);
module_exit(timer_simple_exit);
