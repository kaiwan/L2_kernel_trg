/*
 * kvalloc.c
 * Testing kmalloc, vmalloc limits...
 *
 * Author: Kaiwan N Billimoria
 *
 * Released under the terms of the MIT License.
 *  https://en.wikipedia.org/wiki/MIT_License
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include "convenient.h"

#define	DRVNAME		"kvalloc"

/* 
This kernel module also demos the common technique of having a global "(device) context" 
structure, whose pointer is passed around.
This also has the advantage of, when debugging, getting to all relevant info 
via a single pointer!

Our context structure */
typedef struct _ST_CTX {
	u32 kvalloc; 			 // # bytes to alloc (in)
	u32 *vm;			 // address allocated at (out)
	char * kname, *vname;
	struct mutex lock;
} ST_CTX, *PST_CTX;
static PST_CTX gpstCtx=NULL;

/* 
Poke some mem locations.
Called with the mutex held!
*/
static int do_something(void *kvm, u32 len)
{
	void *x;
	if (!kvm || !len)
		return -1;

	for (x=kvm; x < (kvm+len); x += PAGE_SIZE) {
		//MSG("x=%08x\n", (u32)x);
		memset (x, 0xff, 0x10);
	}

	return 0;
}


//---------------------------- VMALLOC_TEST ------------------------------------------
/* 
vmalloc_test Read callback: just display the last # bytes attempting to allocate...
*/
static int vmalloc_procread(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	PST_CTX pstCtx = (PST_CTX)data;
	int n;

	mutex_lock (&pstCtx->lock);
	*eof = 1;
	n = sprintf (buf, 
		"[%s] Last attempted to allocate via vmalloc: %u bytes (%u Kb, %u MB).\n", 
			DRVNAME, pstCtx->kvalloc, pstCtx->kvalloc/1024, pstCtx->kvalloc/(1024*1024));
	mutex_unlock (&pstCtx->lock);
	return n;
}

/*
vmalloc_test Write callback:
Userspace expected to write to this proc entry, the number of bytes to
attempt to allocate.
*/
static int vmalloc_procwrite(struct file *file, const char __user *buffer,
                           unsigned long count, void *data)
{
	PST_CTX pstCtx = (PST_CTX)data;
	int sz=18, /* # of digits (=bytes written in) */ status=count;
	char kbuf[sz+1];

//MSG("count=%d\n", count);
	mutex_lock (&pstCtx->lock);

	if (count > sz) {
		printk("%s:%s: writing a number > %d digits is invalid\n", DRVNAME, __func__, sz);
		status = -EINVAL;
		goto out_inval;
	}
	if (copy_from_user (kbuf, buffer, sz)) {
		printk("%s: copy_from_user() failed!\n", DRVNAME);
		status = -EIO;
		goto out_inval;
	}
	kbuf[sz+1]='\0';

	pstCtx->kvalloc = simple_strtoul(kbuf, NULL, 10);
	if (0 == pstCtx->kvalloc) {
		printk("%s:%s: writing invalid numeric value \"%.*s\", aborting...\n", DRVNAME, __func__, (int)count, kbuf);
		status = -EINVAL;
		goto out_inval;
	}
	//MSG("kvalloc=%u\n", kvalloc);

	pstCtx->vm = vmalloc (pstCtx->kvalloc);
	if (!pstCtx->vm) {
		printk (KERN_INFO "%s: vmalloc of %u bytes FAILED!\n", DRVNAME, pstCtx->kvalloc);
		status = -ENOMEM;
		goto out_inval;
	}
	MSG ("Successfully allocated via vmalloc %u bytes (%u Kb, %u MB) now to location 0x%08x (will vfree..)\n", 
			pstCtx->kvalloc, pstCtx->kvalloc/1024, pstCtx->kvalloc/(1024*1024), (u32)pstCtx->vm);

	do_something(pstCtx->vm, pstCtx->kvalloc);
	vfree (pstCtx->vm);

out_inval:
	mutex_unlock (&pstCtx->lock);
	return status;
}

//---------------------------- KMALLOC_TEST ------------------------------------------
/* 
kmalloc_test Read callback: just display the last # bytes attempting to allocate...
*/
static int kmalloc_procread(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	PST_CTX pstCtx = (PST_CTX)data;
	int n;

	mutex_lock (&pstCtx->lock);
	*eof = 1;
	n = sprintf (buf, 
		"[%s] Last attempted to allocate via kmalloc: %u bytes (%u Kb, %u MB).\n", 
			DRVNAME, pstCtx->kvalloc, pstCtx->kvalloc/1024, pstCtx->kvalloc/(1024*1024));
	mutex_unlock (&pstCtx->lock);
	return n;
}

/*
kmalloc_test Write callback:
Userspace expected to write to this proc entry, the number of bytes to
attempt to allocate.
*/
static int kmalloc_procwrite(struct file *file, const char __user *buffer,
                           unsigned long count, void *data)
{
	PST_CTX pstCtx = (PST_CTX)data;
	int sz=9, /* # of digits (=bytes written in) */ status=count;
	char kbuf[sz+1];

	mutex_lock (&pstCtx->lock);

	if (count > sz) {
		printk("%s:%s: writing a number > %d digits is invalid\n", DRVNAME, __func__, sz);
		status = -EINVAL;
		goto out_inval;
	}
	if (copy_from_user (kbuf, buffer, sz)) {
		printk("%s: copy_from_user() failed!\n", DRVNAME);
		status = -EIO;
		goto out_inval;
	}
	kbuf[sz+1]='\0';

	pstCtx->kvalloc = simple_strtoul(kbuf, NULL, 10);
	if (0 == pstCtx->kvalloc) {
		printk("%s:%s: writing invalid numeric value \"%.*s\", aborting...\n", DRVNAME, __func__, (int)count, kbuf);
		status = -EINVAL;
		goto out_inval;
	}
	//MSG("kvalloc=%u\n", kvalloc);

	pstCtx->vm = kmalloc (pstCtx->kvalloc, GFP_KERNEL);
	if (!pstCtx->vm) {
		printk (KERN_INFO "%s: kmalloc of %u bytes FAILED!\n", DRVNAME, pstCtx->kvalloc);
		status = -ENOMEM;
		goto out_inval;
	}
	MSG ("Successfully allocated via kmalloc %u bytes (%u Kb, %u MB) now to location 0x%08x (will kfree..)\n", 
			pstCtx->kvalloc, pstCtx->kvalloc/1024, pstCtx->kvalloc/(1024*1024), (u32)pstCtx->vm);

	do_something(pstCtx->vm, pstCtx->kvalloc);
	kfree (pstCtx->vm);

out_inval:
	mutex_unlock (&pstCtx->lock);
	return status;
}

static struct proc_dir_entry *res;
static int __init vmall_init_module(void)
{
	int status=0;

	//-------- Alloc & init global context struct
	gpstCtx = kzalloc (sizeof(ST_CTX), GFP_KERNEL);
	if (!gpstCtx) {
		MSG("%s: kmalloc 1 failed, aborting..\n", DRVNAME);
		status = -ENOMEM;
		goto out_k1_fail;
	}

	gpstCtx->kname = kmalloc (128, GFP_KERNEL);
	if (!gpstCtx->kname) {
		MSG("%s: kmalloc 2 failed, aborting..\n", DRVNAME);
		status = -ENOMEM;
		goto out_k2_fail;
	}
	strncpy (gpstCtx->kname, "driver/kmalloc_test", 128);

	gpstCtx->vname = kmalloc (128, GFP_KERNEL);
	if (!gpstCtx->vname) {
		MSG("%s: kmalloc 3 failed, aborting..\n", DRVNAME);
		status = -ENOMEM;
		goto out_k3_fail;
	}
	strncpy (gpstCtx->vname, "driver/vmalloc_test", 128);
	mutex_init (&gpstCtx->lock);

	//-------- Setup proc entry points
	res = create_proc_entry (gpstCtx->kname, 0644, /* mode */ NULL);
	if (!res) {
		MSG("%s: Entry %s creation failure, aborting..\n", DRVNAME, gpstCtx->kname);
		status = -ENOMEM;
		goto out_p1_fail;
	}
	res->read_proc = kmalloc_procread;
	res->write_proc = kmalloc_procwrite;
	res->data = gpstCtx;

	res = create_proc_entry (gpstCtx->vname, 0644, /* mode */ NULL);
	if (!res) {
		MSG("%s: Entry %s creation failure, aborting..\n", DRVNAME, gpstCtx->vname);
		status = -ENOMEM;
		goto out_p2_fail;
	}
	res->read_proc = vmalloc_procread;
	res->write_proc = vmalloc_procwrite;
	res->data = gpstCtx;

	MSG("Loaded ok.\n");
	return 0;

out_p2_fail:
	remove_proc_entry (gpstCtx->kname, NULL);
out_p1_fail:
	kfree (gpstCtx->vname);
out_k3_fail:
	kfree (gpstCtx->kname);
out_k2_fail:
	kfree (gpstCtx);
out_k1_fail:
	return status;
}

static void __exit vmall_cleanup_module(void)
{
	remove_proc_entry (gpstCtx->vname, NULL);
	remove_proc_entry (gpstCtx->kname, NULL);
	kfree (gpstCtx->vname);
	kfree (gpstCtx->kname);
	kfree (gpstCtx);
	MSG("Removed.\n");
}

/*----------------------------- Module stuff */
module_init(vmall_init_module);
module_exit(vmall_cleanup_module);
MODULE_AUTHOR("Kaiwan NB <kaiwan -at- kaiwantech dot com>");
MODULE_DESCRIPTION("Testing kmalloc / vmalloc limits");
MODULE_LICENSE("Dual MIT/GPL");
