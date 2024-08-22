/*
 * showthreads.c
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * MIT/GPL.
 */
//#define pr_fmt(fmt) "%s:%s():%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif
#include <linux/sched.h>

#define	DRVNAME		"showthreads"

static void showthrds(void)
{
	struct task_struct *g, *t;	// 'g' : process ptr; 't': thread ptr !
	char buf[256], tmp[128];

	pr_info
	    ("----------------------------------------------------------------------------\n"
	     "    TGID   PID               Thread Name        # Threads\n"
	     "----------------------------------------------------------------------------\n");

	rcu_read_lock();
//	do_each_thread(g, t) {
	/* the do_each_thread() { ... } while_each_thread() seems to throw a warning-treated-as-error:
howthreads.c:30:9: error: implicit declaration of function ‘do_each_thread’; did you mean ‘for_each_thread’? [-Werror=implicit-function-declaration]
   30 |         do_each_thread(g, t) {
      |         ^~~~~~~~~~~~~~
	 * ... 
	 * So lets just use the simpler form: for_each_process_thread()
	 */
	for_each_process_thread(g, t) {
		int nr_thrds = 1;

		task_lock(t);
		get_task_struct(t);

		snprintf(buf, 256, "%6d %6d ", g->tgid, t->pid);
		if (!g->mm) {	// kernel thread
			snprintf(tmp, 128, "[%30s]", t->comm);
		} else {
			snprintf(tmp, 128, " %30s ", t->comm);
		}
		strncat(buf, tmp, 128);

		nr_thrds = get_nr_threads(g);
		// "main" thread of a multithread app?
		if (g->mm && (g->tgid == t->pid) && (nr_thrds > 1)) {
			snprintf(tmp, 128, "    %4d", nr_thrds);
			strncat(buf, tmp, 128);
		}

		snprintf(tmp, 2, "\n");
		strncat(buf, tmp, 2);
		pr_info("%s", buf);

		memset(buf, 0, sizeof(buf));
		memset(tmp, 0, sizeof(tmp));
		put_task_struct(t);
		task_unlock(t);
	}
	//while_each_thread(g, t);
	rcu_read_unlock();
}

static int __init showthrds_init_module(void)
{
	showthrds();
	return 0;
}

static void __exit showthrds_cleanup_module(void)
{
	pr_info("%s: unoaded.\n", KBUILD_MODNAME);
}

module_init(showthrds_init_module);
module_exit(showthrds_cleanup_module);

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("Displays all threads.");
MODULE_LICENSE("GPL");
//MODULE_LICENSE("Dual GPL/MIT");
