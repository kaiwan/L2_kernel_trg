#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "../../../convenient.h"

MODULE_LICENSE("Dual MIT/GPL");

static int __init kmt_init(void)
{
	int num=100;
	void *p;
	while (1) {
		p = kmalloc(num, GFP_KERNEL);
		if (!p) {
			pr_alert("kmalloc fail, num=%d\n", num);
			return -ENOMEM;
		}
		num += 1000;
		pr_info("kmalloc(%7d) = %p\n", num, p);
		kfree(p);
	}
	return 0; // success
}

static void __exit kmt_exit(void)
{
	pr_info("Goodbye\n");
}

module_init(kmt_init);
module_exit(kmt_exit);
