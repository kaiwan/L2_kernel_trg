/*
 * lkm_template/lkm_template.c
 ***************************************************************
 * < Your desc, comments, etc >
 * (c) Author:
 ****************************************************************
 * Brief Description:
 *
 */
#include <linux/init.h>
#include <linux/module.h>

#define OURMODNAME   "lkm_template"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("a hello, world type LKM");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static int __init lkm_template_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	return 0;		/* success */
}

static void __exit lkm_template_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(lkm_template_init);
module_exit(lkm_template_exit);
