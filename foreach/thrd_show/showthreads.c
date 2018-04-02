/*
 * showthreads.c
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * MIT/GPL.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0) 
#include <linux/sched/signal.h>
#endif

#define	DRVNAME		"showthreads"

static void showthrds(void)
{
	struct task_struct *g, *t; // 'g' : process ptr; 't': thread ptr !
	int nr_thrds=1;
	char buf[256], tmp[128];

	pr_info(
"----------------------------------------------------------------------------\n"
"    TGID   PID               Thread Name        # Threads\n"
"----------------------------------------------------------------------------\n"
	);

	do_each_thread(g, t) {
		task_lock(t);

		snprintf(buf, 256, "%6d %6d ", g->tgid, t->pid);
		if (!g->mm) { // kernel thread
			 snprintf(tmp, 128, "[%30s]", t->comm);
		} else {
			 snprintf(tmp, 128, " %30s ", t->comm);
		}
		strncat(buf, tmp, 128);

		nr_thrds = get_nr_threads(g);
		// "main" thread of multiple
		if (g->mm && (g->tgid == t->pid) && (nr_thrds > 1)) {
			snprintf(tmp, 128, "    %4d", nr_thrds);
			strncat(buf, tmp, 128);
		}

		snprintf(tmp, 2, "\n");
		strncat(buf, tmp, 2);
		pr_info("%s", buf);

		memset(buf, 0, sizeof(buf));
		memset(tmp, 0, sizeof(tmp));
		task_unlock(t);
	} while_each_thread(g, t);
}

static int __init showthrds_init_module(void)
{
	showthrds();
	return 0;
}

static void __exit showthrds_cleanup_module(void)
{
	pr_info("%s: unoaded.\n", DRVNAME);
}

module_init(showthrds_init_module);
module_exit(showthrds_cleanup_module);

MODULE_AUTHOR("Kaiwan NB");
MODULE_DESCRIPTION("Displays all threads.");
MODULE_LICENSE("GPL");
//MODULE_LICENSE("Dual GPL/MIT");
