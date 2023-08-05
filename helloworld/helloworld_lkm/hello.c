#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("Dual MIT/GPL");

static int __init hello_init(void)
{
	pr_alert("Hello, LKM world\n");

#if 0
	return -ENOMEM;
/*
 * If we do this, insmod fails immd with:
 * insmod: ERROR: could not insert module ./hello.ko: Cannot allocate memory
 * The cleanup/exit handler is *not* invoked
 */
#endif

#if 1
	return 0;		// success
#endif
}

static void __exit hello_exit(void)
{
	pr_alert("Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
