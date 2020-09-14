#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>  // for current

MODULE_LICENSE("Dual MIT/GPL");

static int __init hello_init(void)
{
	pr_alert("init: Process context :: %s PID %d\n", 
				current->comm, current->pid);
#if 0
    dump_stack();
#endif
	return 0; // success
}

static void __exit hello_exit(void)
{
	pr_alert("exit: Process context :: %s PID %d\n", 
				current->comm, current->pid);
#if 0
    dump_stack();
#endif
}

module_init(hello_init);
module_exit(hello_exit);

