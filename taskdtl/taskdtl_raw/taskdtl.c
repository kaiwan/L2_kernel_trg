/*
 * taskdtl.c
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
#include <linux/sched.h>     /* current */

#include <linux/vmalloc.h>   /* gcc err "dereferencing pointer to incomplete type" if not included */
#include <linux/mm_types.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
#include <linux/sched/signal.h>
#endif

#include <linux/mm.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>

MODULE_LICENSE("Dual MIT/GPL");

#define MODNAME    "taskdtl"
static int pid;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid, "set this to the PID of the process or thread to dump task info about");

static int disp_task_details(struct task_struct *p)
{
	char *pol=NULL;
	struct thread_info *ti = task_thread_info(p);

	task_lock(p);

	/* Task PID, ownership */
	pr_info("Task struct @ 0x%lx ::\n"
		"Process/Thread: %16s, TGID %6d, PID %6d\n"
		"%26sRealUID    : %6d, EffUID : %6d\n"
		"%26slogin UID  : %6d\n",
		p, p->comm, p->tgid, p->pid,
		" ", __kuid_val(p->cred->uid), __kuid_val(p->cred->euid),
		" ", __kuid_val(p->loginuid));

#if 0
#ifdef CONFIG_ARM64
	u64 sp_el0;
	asm ("mrs %0, sp_el0" : "=r" (sp_el0));
	pr_info(" arm64: sp_el0 holds task ptr (value is 0x%lx)\n", sp_el0);
#endif /* CONFIG_ARM64 */
#endif

	/* Task state */
	pr_info("Task state (%d) : ", p->state);
	switch (p->state) {
	case TASK_RUNNING : pr_info(" R: ready-to-run (on rq) OR running (on cpu)\n");
			    break;
	case TASK_INTERRUPTIBLE : pr_info(" S: interruptible sleep\n");
			    break;
	case TASK_UNINTERRUPTIBLE : pr_info(" D: uninterruptible sleep (!)\n");
			    break;
	case __TASK_STOPPED : pr_info(" T: stopped\n");
			    break;
	case __TASK_TRACED : pr_info(" t: traced\n");
			    break;
	case TASK_PARKED : pr_info(" .: PARKED\n");
			    break;
	case TASK_DEAD : pr_info(" X: DEAD\n");
			    break;
	case TASK_WAKEKILL : pr_info(" .: WAKEKILL\n");
			    break;
	case TASK_WAKING : pr_info(" .: WAKING\n");
			    break;
	case TASK_NOLOAD : pr_info(" .: NOLOAD\n");
			    break;
	case TASK_NEW : pr_info(" .: NEW\n");
			    break;
	default : pr_info(" ? <unknown>\n");
	}

	if (!ti)
		pr_warn("ti invalid!\n");
	else {
		pr_info(
		"thread_info (0x%lx) is "
#ifdef CONFIG_THREAD_INFO_IN_TASK
		"within the task struct itself\n"
#else
		"separate (not within the task struct)\n"
#endif
		, ti);
	}

	pr_info(
		"stack      : 0x%lx ; vmapped?"  /* we use the %lx fmt spec instead of the
			     * recommended-for-security %pK as we really do want
			     * to see the actual address here; don't do this in
			     * production! */
#ifdef CONFIG_VMAP_STACK
	" yes\n"
#else
	" no\n"
#endif
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	"  (GCC STACKLEAK) lowest stack : 0x%lx\n"
#endif
	"flags : 0x%x\n"
	"sched ::\n"
#ifdef CONFIG_THREAD_INFO_IN_TASK
	"  curr CPU    : %3d\n"
#endif
	"  on RQ?      : %s\n"
	"  prio        : %3d\n"
	"  static prio : %3d\n"
	"  normal prio : %3d\n"
	"  RT priority : %3d\n"
	"  vruntime    : %llu\n",
	p->stack,
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	p->lowest_stack,
#endif
	p->flags,
#ifdef CONFIG_THREAD_INFO_IN_TASK
	p->cpu,
#endif
	p->on_rq ? "yes" : "no",
	p->prio,
	p->static_prio,
	p->normal_prio,
	p->rt_priority,
	p->se.vruntime
	);

	// Sched Policy
	switch (p->policy) {
	case SCHED_NORMAL: pol = "Normal/Other";
			break;
	case SCHED_FIFO: pol = "FIFO";
			break;
	case SCHED_RR: pol = "RR";
			break;
	case SCHED_BATCH: pol = "BATCH";
			break;
	case SCHED_IDLE: pol = "IDLE";
			break;
	case SCHED_DEADLINE: pol = "DEADLINE";
			break;
	default: pol = "<Unknown>";
	}
	pr_info("  policy      : %s", pol);

	pr_info(
	"  cpus allowed: %3d\n"
#ifdef CONFIG_SCHED_INFO
	"  # times run on cpu: %4ld\n"
	"  time waiting on RQ: %9llu\n"  // TODO :: unit ??
#endif
	,	
	p->nr_cpus_allowed
#ifdef CONFIG_SCHED_INFO
	,
	p->sched_info.pcount,
	p->sched_info.run_delay // TODO :: unit ??
#endif
	);

	/* some mm details */
	if (p->mm) {
		pr_info("mm info ::\n"
			" not a kernel thread; mm_struct   : 0x%lx\n", p->mm);
		pr_info(
		" PGD base addr : 0x%lx\n"
		" mm_users = %d, mm_count = %d\n"
	#ifdef CONFIG_MMU
		" PTE page table pages = %ld bytes\n"
	#endif
		" # of VMAs                                  =  %9d\n"
		" Highest VMA end address                    = 0x%lx\n"
		" High-watermark of RSS usage                = %9lu pages\n"
                " High-water virtual memory usage            = %9lu pages\n"
                " Total pages mapped                         = %9lu pages\n"
                " Pages that have PG_mlocked set             = %9lu pages\n"
                " Refcount permanently increased             = %9lu pages\n"
                " data_vm: VM_WRITE & ~VM_SHARED & ~VM_STACK = %9lu pages\n"
                " exec_vm:   VM_EXEC & ~VM_WRITE & ~VM_STACK = %9lu pages\n"
                " stack_vm:                         VM_STACK = %9lu pages\n"
                " def_flags                                  = 0x%x\n"
		,
			p->mm->pgd,
			atomic_read(&p->mm->mm_users),
			atomic_read(&p->mm->mm_count),
		#ifdef CONFIG_MMU
			atomic64_read(&p->mm->pgtables_bytes),
		#endif
			p->mm->map_count,
			p->mm->highest_vm_end,
			p->mm->hiwater_rss, /* High-watermark of RSS usage */
			p->mm->hiwater_vm,  /* High-water virtual memory usage */
			p->mm->total_vm,    /* Total pages mapped */
			p->mm->locked_vm,   /* Pages that have PG_mlocked set */
			p->mm->pinned_vm,   /* Refcount permanently increased */
			p->mm->data_vm,     /* VM_WRITE & ~VM_SHARED & ~VM_STACK */
			p->mm->exec_vm,     /* VM_EXEC & ~VM_WRITE & ~VM_STACK */
			p->mm->stack_vm,    /* VM_STACK */
			p->mm->def_flags
		);

		/* userspace mappings */
		//spin_lock(&p->mm->arg_lock);
		pr_info(
		"mm userspace mapings (high to low) ::\n"
		" env        : 0x%lx - 0x%lx  [%6u bytes]\n"
		" args       : 0x%lx - 0x%lx  [%6u bytes]\n"
	//	" stack      : 0x%lx - 0x%lx  [%6u bytes]\n"
		" start stack: 0x%lx\n"
		" heap       : 0x%lx - 0x%lx  [%6u KB, %6u MB]\n"
		" data       : 0x%lx - 0x%lx  [%6u KB, %6u MB]\n"
		" code       : 0x%lx - 0x%lx  [%6u KB, %6u MB]\n"
		,
		p->mm->env_start, p->mm->env_end,
		p->mm->env_end - p->mm->env_start,
		p->mm->arg_start, p->mm->arg_end,
		p->mm->arg_end - p->mm->arg_start,
		//p->mm->start_stack, p->mm->arg_start,
		//p->mm->arg_start - p->mm->start_stack,
		p->mm->start_stack,
		p->mm->start_brk, p->mm->brk,
		(p->mm->brk - p->mm->start_brk)/1024,
		(p->mm->brk - p->mm->start_brk)/(1024*1024),
		p->mm->start_data, p->mm->end_data,
		(p->mm->end_data - p->mm->start_data)/1024,
		(p->mm->end_data - p->mm->start_data)/(1024*1024),
		p->mm->start_code, p->mm->end_data,
		(p->mm->end_data - p->mm->start_code)/1024,
		(p->mm->end_data - p->mm->start_code)/(1024*1024)
		);
		//spin_unlock(&p->mm->arg_lock);
		// Get SP
#ifdef CONFIG_ARM64
		{
		u64 sp_el1;
		asm ("mrs %0, sp_el1" : "=r" (sp_el1));
		pr_info(" arm64: sp_el1 = 0x%lx\n", sp_el1);
		}
#endif /* CONFIG_ARM64 */
	
	} else
		pr_info("is a kernel thread (mm NUL)\n");

	pr_info(
	"in execve()? %s\n"
	"in iowait  ? %s\n"
	"stack canary  : 0x%lx\n"
	"utime, stime  : %llu, %llu\n"
	"# vol c/s, # invol c/s : %6lu, %6lu\n"
	"# minor, major faults  : %6lu, %6lu\n"
#ifdef CONFIG_LOCKDEP
	"lockdep depth : %d\n"
#endif
#ifdef CONFIG_TASK_IO_ACCOUNTING
	"task I/O accounting ::\n"
	" read bytes               : %7llu\n"
	" written (or will) bytes  : %7llu\n"
	" cancelled write bytes    : %7llu\n"
#endif
#ifdef CONFIG_TASK_XACCT
	" # read syscalls          : %7llu\n"
	" # write syscalls         : %7llu\n"
	" accumulated RSS usage    : %9llu (%6llu KB)\n"  // bytes ??
	" accumulated  VM usage    : %9llu (%6llu KB)\n"  // bytes ??
#endif
#ifdef CONFIG_PSI
	"pressure stall state flags: 0x%lx\n"
#endif
	,
	p->in_execve == 1 ? "yes" : "no",
	p->in_iowait == 1 ? "yes" : "no",
	p->stack_canary,
	p->utime, p->stime,
	p->nvcsw, p->nivcsw,
	p->min_flt, p->maj_flt,
#ifdef CONFIG_LOCKDEP
	p->lockdep_depth,
#endif
#ifdef CONFIG_TASK_IO_ACCOUNTING
	p->ioac.read_bytes,
	p->ioac.write_bytes,
	p->ioac.cancelled_write_bytes,
#endif
#ifdef CONFIG_TASK_XACCT
	p->ioac.syscr,
	p->ioac.syscw,
	p->acct_rss_mem1, p->acct_rss_mem1/1024,
	p->acct_vm_mem1, p->acct_vm_mem1/1024
#endif
#ifdef CONFIG_PSI
	,
	p->psi_flags
#endif
	);

	/* thread struct : cpu-specific hardware context */
	pr_info(
	"Hardware ctx info location is thread struct: 0x%lx\n"
#ifdef CONFIG_X86_64
	"X86_64 ::\n"
	" thrd info: 0x%lx\n"
	"  sp  : 0x%lx\n"
	"  es  : 0x%x, ds  : 0x%x\n"
	"  cr2 : 0x%lx, trap #  : 0x%lx, error code  :  0x%lx\n"
	"  mm: addr limit (user boundary) : 0x%lx (%lu GB, %lu TB)\n" /* %lu EB)\n" */
#endif /* CONFIG_X86_64 */
#ifdef CONFIG_ARM64
	"ARM64 ::\n"
	" thrd info: 0x%lx\n"
	"  addr limit : 0x%lx (%u MB, %u GB)\n"
	" Saved registers ::\n"
	"  X19 = 0x%lx  X20 = 0x%lx   X21 = 0x%lx\n"
	"  X22 = 0x%lx  X23 = 0x%lx   X24 = 0x%lx\n"
	"  X25 = 0x%lx  X26 = 0x%lx   X27 = 0x%lx\n"
	"  X28 = 0x%lx\n"
	"   FP = 0x%lx   SP = 0x%lx    PC = 0x%lx\n"
	"  fault_address  : 0x%lx, fault code (ESR_EL1): 0x%lx\n"
	" arm64 pointer authentication"
 #ifdef CONFIG_ARM64_PTR_AUTH
	" present.\n"
 #else
	" absent.\n"
 #endif
#endif /* CONFIG_ARM64 */
	,
	&(p->thread),
#ifdef CONFIG_X86_64
	ti,
	p->thread.sp,
	p->thread.es, p->thread.ds,
	p->thread.cr2, p->thread.trap_nr, p->thread.error_code,
	p->thread.addr_limit.seg, (unsigned long)p->thread.addr_limit.seg/(1024*1024),
	(unsigned long)p->thread.addr_limit.seg/(1024*1024*1024)
	/* (unsigned long)p->thread.addr_limit.seg/(1024*1024*1024*1024) : results in IoF ! */
#endif /* CONFIG_X86_64 */
#ifdef CONFIG_ARM64
	ti,
	ti->addr_limit,
	ti->addr_limit/(1024*1024),
	ti->addr_limit/(1024*1024*1024),
	p->thread.cpu_context.x19, p->thread.cpu_context.x20, p->thread.cpu_context.x21,
	p->thread.cpu_context.x22, p->thread.cpu_context.x23, p->thread.cpu_context.x24,
	p->thread.cpu_context.x25, p->thread.cpu_context.x26, p->thread.cpu_context.x27,
	p->thread.cpu_context.x28,
	p->thread.cpu_context.fp, p->thread.cpu_context.sp, p->thread.cpu_context.pc,
	p->thread.fault_address, p->thread.fault_code
#endif /* CONFIG_ARM64 */
	);

	task_unlock(p);
#if 0
    dump_stack();
#endif
	return 0;
}

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

module_init(taskdtl_init);
module_exit(taskdtl_exit);
