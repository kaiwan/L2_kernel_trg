/*
 * taskdtl_miscdrv.c
 ****************************************************************
 * Brief Description:
 * This kernel module, given the PID of a thread/process as parameter,
 * gains access to it's task structure and prints some relevant info
 * from it to the kernel log (includes some arch-specific detail as well).
 *
 * Author: (c) Kaiwan N Billimoria <kaiwan dot billimoria -at- gmail dot com>
 * License: MIT / GPL
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// 'misc' driver stuff
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include "../../convenient.h"

MODULE_LICENSE("Dual MIT/GPL");

#define MODNAME    "taskdtl_miscdrv"
int disp_task_details(struct task_struct *);

static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("%s:%s() called.\n"
		" minor # is %d\n",
		MODNAME, __func__, iminor(inode));
	return 0;
}

static ssize_t my_dev_read(struct file *file, char __user *buf,
	size_t count, loff_t *off)
{
	pr_info("%s:%s() called.\n"
		" request to read %zu bytes\n",
		MODNAME, __func__, count);
	return count;
}

static ssize_t my_dev_write(struct file *file, const char __user *buf,
	size_t count, loff_t *off)
{
	struct task_struct *tp = NULL;
	char *kbuf = kzalloc(10, GFP_KERNEL);
	long pid = 1;
	ssize_t status = count;

	PRINT_CTX();
	pr_debug("%s:%s() called;\n"
		" request to write %zu bytes\n",
		MODNAME, __func__, count);
	if (!kbuf) {
		pr_err("%s:%s(): kzalloc (10) failed !\n", MODNAME, __func__);
		return -ENOMEM;
	}
	
	// Read the PID written from userspace
	if (copy_from_user(kbuf, buf, count)) {
		pr_err("%s:%s(): copy_from_user failed !\n", MODNAME, __func__);
		status = -EFAULT;
		goto out1;
	}
	if ((status = kstrtol(kbuf, 0, &pid)) < 0) {
		pr_err("%s:%s(): kstrtol failed : was a numeric value passed?\n",
			MODNAME, __func__);
		goto out1;
	}
	status = count;

	tp = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!tp) {
		pr_err("%s: failed to obtain task struct pointer for PID %d\n",
			MODNAME, pid);
		status = -EINVAL;
		goto out1;
	}
	pr_debug(" pid=%d, tp=0x%lx\n", pid, tp);
	disp_task_details(tp);

out1:
	kfree(kbuf);
	return status;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("%s:%s() called.\n", MODNAME, __func__);
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("%s:%s() called.\n"
		" cmd = %d, arg = %ld\n"
		, MODNAME, __func__, cmd, arg);
	return 0;
}

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.read = my_dev_read,
	.write = my_dev_write,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

/* declare & initialize struct miscdevice */
static struct miscdevice my_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR, /* dynamically allocate an available minor # */
	.name = "mydev",
	.fops = &my_dev_fops,        /* functionality */
};


static int __init taskdtl_init(void)
{
	int ret_val;
	pr_info("%s: taskdtl misc driver init\n", MODNAME);

	/* Register the device with the Kernel */
	ret_val = misc_register(&my_miscdevice);
	if (ret_val != 0) {
		pr_err("%s: could not register the misc device mydev", MODNAME);
		return ret_val;
	}

	pr_info("%s:minor=%d\n", MODNAME, my_miscdevice.minor);
	pr_info("%s: initialized\n", MODNAME);
	return 0;
}

static void __exit taskdtl_exit(void)
{
	pr_info("%s: exiting now\n", MODNAME);
	/* Unregister the device with the kernel */
	misc_deregister(&my_miscdevice);
}

#if 0
static int __init taskdtl_init(void)
{
	struct task_struct *tp;
	
	if (0 == pid) {
		pr_err("%s: pl pass a valid PID as parameter to this kernel module\n", MODNAME);
		return -EINVAL;
	}
	tp = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!tp) {
		pr_err("%s: failed to obtain task struct pointer for PID %d\n",
			MODNAME, pid);
		return -EINVAL;
	}
	pr_debug("pid=%d, tp = 0x%lx\n", pid, tp);
	disp_task_details(tp);
	return 0;
}
static void __exit taskdtl_exit(void)
{
	pr_info("%s: exiting now\n", MODNAME);
#if 0
    dump_stack();
#endif
}
#endif

module_init(taskdtl_init);
module_exit(taskdtl_exit);
