/*
 * foreach.c
 *
 * Simple kernel program that -
 * -returns info about all *processes* (_not_ threads) on the task list
 * 
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * Released under the terms of the MIT License.
 *  https://en.wikipedia.org/wiki/MIT_License
 */
#define pr_fmt(fmt) "%s:%s():%d: " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>	/* current, jiffies */

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <linux/sched/signal.h>	/* for_each_xxx, ... */
#endif
#include <linux/fs.h>		/* no_llseek */
#include <linux/slab.h>
#include <linux/uaccess.h>	/* copy_to_user() */
#include <linux/kallsyms.h>

#define	DRVNAME		"foreach"
#define MY_MAJOR  	0    /* 0 => dynamic major number assignment */
#define MAXKBUF_LEN	20*1024 // 20Kb should do.... ?

static int taskinfo_major = MY_MAJOR;

static ssize_t taskinfo_read(struct file *filp, char __user *buf, 
		size_t count, loff_t *offp)
{
	struct task_struct *p;
	char *kbuf, tmp[128];
	int numread=0, num=0;

	if (!(kbuf = kzalloc(MAXKBUF_LEN, GFP_KERNEL))) {
		pr_warn("kmalloc failed");
		return -ENOMEM;
	}

	rcu_read_lock();
	for_each_process(p) {
		memset (tmp, 0, 128);
		task_lock(p);
		get_task_struct(p);
		num = snprintf (tmp, 128, "\
%-16s|%7d|%7d|%7u|%7u\n",
			p->comm, p->tgid, p->pid,
			//p->uid, p->euid
			//current_uid().val, current_euid().val
			__kuid_val(p->cred->uid),
			__kuid_val(p->cred->euid)
		);
		strncat (kbuf, tmp, num);
		numread += num;
		pr_debug("num=%d numread=%d tmp=%s\n", num, numread, tmp);

		/* latency reduction via explicit rescheduling in places that are safe */
		if (cond_resched())
			pr_debug("resched occured now\n");

		put_task_struct(p);
		task_unlock(p);
	} // for_each_process loop...
	rcu_read_unlock();

//printk("\n\n%s\n", kbuf);

#if 1
	if (copy_to_user(buf, kbuf, numread)) {
		pr_alert("copy_to_user failed..\n");
		kfree(kbuf);
		return -EFAULT;
	}
#endif
	kfree(kbuf);
	return count;
}

static ssize_t taskinfo_write(struct file *filp, const char __user *buf, 
		size_t count, loff_t *offp)
{
	pr_debug("write isn't supported. [fyi, process %s [pid %d], count=%ld]\n",
			current->comm, current->pid, count);
	return -ENOSYS;
}


/* Minor-specific open routines */
static struct file_operations taskinfo_fops = {
	.llseek =	no_llseek,
	.read =		taskinfo_read,	// do this with ioctl..better.
	.write =	taskinfo_write,
};

static int taskinfo_open(struct inode * inode, struct file * filp)
{
	pr_debug("Device node with minor # %d being used\n", iminor(inode));

	switch (iminor(inode)) {
		case 0:
			filp->f_op = &taskinfo_fops;
			break;
		default:
			return -ENXIO;
	}
	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode,filp); 
		   /* Minor-specific open : jumps to the 
		   suitable entry point - the correct open() call if 
	      	   one has been defined */
	return 0;
}

/* Major-wide open routine */
static struct file_operations taskinfoopen_fops = {
	.open =		taskinfo_open, /* just a means to get at the real open */
};

/*
 * Register the char driver with the kernel.
 *
 *  On 2.6 kernels, we could use the new alloc_register_chrdev() function; 
 *  here, we use the "classic" register_chrdev() API.
 *	result = register_chrdev( MY_MAJOR, "cz", &cz_fops );
 */
static int __init taskinfo_init_module(void)
{
	int result;

	pr_debug("taskinfo_major=%d\n",taskinfo_major);

	/*
	 * Register the major, and accept a dynamic number.
	 * The return value is the actual major # assigned.
	 */
	result = register_chrdev(taskinfo_major, DRVNAME, &taskinfoopen_fops);
	if (result < 0) {
		pr_debug("register_chrdev() failed trying to get taskinfo_major=%d\n",
			taskinfo_major);
		return result;
	}

	if (taskinfo_major == 0) taskinfo_major = result; /* dynamic */
	pr_info( "registered:: taskinfo_major=%d\n",taskinfo_major);

	return 0; /* success */
}

static void __exit taskinfo_cleanup_module(void)
{
	unregister_chrdev(taskinfo_major, DRVNAME);
	pr_info("Unregistered.\n");
}

module_init(taskinfo_init_module);
module_exit(taskinfo_cleanup_module);

module_param(taskinfo_major, int, 0); /* 0 => param won't show up in sysfs, 
				   non-zero are mode (perms) */
MODULE_PARM_DESC(taskinfo_major, "Major number to attempt to use");
MODULE_AUTHOR("Kaiwan");
MODULE_DESCRIPTION("Simple module that returns task info to userspace");
MODULE_LICENSE("Dual MIT/GPL");
