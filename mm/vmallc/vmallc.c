#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("Dual MIT/GPL");

static void *vp;
static int __init hello_init(void)
{
	vp = vmalloc(8*PAGE_SIZE);
	//memset(vp, 0, 8*1024*1024);
	return 0;		// success
}

static void __exit hello_exit(void)
{
	vfree(vp);
	pr_info("%s removed\n", KBUILD_MODNAME);
}

module_init(hello_init);
module_exit(hello_exit);
