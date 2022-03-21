/*
 * ksz_test.c
 * A quick kmalloc/ksize test; we allocate kernel RAM with kmalloc(),
 * starting with 100 bytes and incrementing this in a loop by 'stepsz'
 * bytes (you can vary it by passing it as a module parameter).
 * Thus, this kernel module clearly shows how kernel memory can get wasted
 * by using poorly thought-out amounts.
 *
 * Note- the kernel module will probably appear to fail on insmod with a
 * message similar to:
# insmod ./ksz_test.ko
insmod: ERROR: could not insert module ./ksz_test.ko: Cannot allocate memory
#
 * This is expected- we stress the system asking for larger and larger chunks
 * of kernel direct-mapped RAM via the kmalloc, so ultimately it will fail.
 *
 * Please LOOK UP the kernel ring buffer via 'dmesg' to see the printk's!
 *
 * Author: Kaiwan NB, kaiwanTECH
 * License: Dual MIT/GPL
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Kaiwan NB, kaiwanTECH");
MODULE_DESCRIPTION
("A quick kmalloc/ksize test; shows how kernel memory can get wasted"
" by using poorly thought-out amounts");

static int stepsz = 10000;
module_param(stepsz, int, 0);
MODULE_PARM_DESC(stepsz,
		 "Number of bytes to increment each kmalloc attempt by (default=10000)");

const char *hdr =
"[1] kmalloc(n)   :[2]actual: wastage : %age\n"
"      bytes      :   bytes :  [2-1]  : waste\n";

static int __init kmt_init(void)
{
	int num = 100, actual_alloc = 0;
	void *p;

	pr_info("%s", hdr);

	while (1) {
		p = kmalloc(num, GFP_KERNEL);
		if (unlikely(!p)) {
			pr_alert("kmalloc fail, num=%d\n", num);
			return -ENOMEM;
		}
		actual_alloc = ksize(p);
		pr_info("kmalloc(%7d) : %7d : %7d : %3d%%\n",
			num, actual_alloc, actual_alloc - num,
			(((actual_alloc - num) * 100 / num)));
		kfree(p);
		num += stepsz;
	}
	return 0;		// success
}

static void __exit kmt_exit(void)
{
	pr_info("Goodbye\n");
}

module_init(kmt_init);
module_exit(kmt_exit);
