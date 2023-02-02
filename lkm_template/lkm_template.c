/*
 * lkm_template/lkm_template.c
 ***************************************************************
 * < Your desc, comments, etc >
 * (c) Author:
 ****************************************************************
 * Brief Description:
 * (A 'Hello, world' type Loadable Kernel Module (LKM) template, as such!)
 *
 * License: Dual MIT/GPL
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("a hello, world type LKM");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static int __init lkm_template_init(void)
{
	pr_info("inserted\n");
#ifdef DEBUG
	pr_info("DEBUG defined\n");
	pr_debug("this pr_debug() will work!\n");
#else
	pr_info("DEBUG undefined\n");
	pr_debug("this pr_debug() won't work!\n");
#endif
	return 0;		/* success */
}

static void __exit lkm_template_exit(void)
{
	pr_info("removed\n");
}

module_init(lkm_template_init);
module_exit(lkm_template_exit);
