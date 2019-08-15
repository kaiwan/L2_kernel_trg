#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>     // current
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
#include <linux/sched/signal.h>
#endif

MODULE_LICENSE("Dual MIT/GPL");

static int disp_task_details(void)
{
	struct task_struct *p = current;
	char *pol=NULL;

	task_lock(p);
	pr_info("Process / Thread :: %s : TGID %d, PID %d\n"
		" RealUID : %d, EffUID : %d\n",
		p->comm, p->tgid, p->pid,
		__kuid_val(p->cred->uid), __kuid_val(p->cred->euid));

	pr_info(
	" thread_info is "
#ifdef CONFIG_THREAD_INFO_IN_TASK
	"within the task struct itself\n"
#else
	"separate (not within the task struct)\n"
#endif
	" state : %ld\n"

	" stack : 0x%lx\n"  /* we use the %lx fmt spec instead of the
			     * recommended-for-security %pK as we really do want
			     * to see the actual address here; don't do this in
			     * production! */
#ifdef CONFIG_VMAP_STACK
	"  stack is vmap-ped (addr : 0x%lx)\n"
#endif
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	"   (GCC STACKLEAK) lowest stack : 0x%lx\n"
#endif
	" flags : 0x%x\n"
#ifdef CONFIG_THREAD_INFO_IN_TASK
	" curr CPU : 0x%x\n"
#endif
	" sched ::\n"
	"  on RQ?      : %s\n"
	"  prio        : %3d\n"
	"  static prio : %3d\n"
	"  normal prio : %3d\n"
	"  RT priority : %3d\n"
	"  vruntime    : %llu\n",
	p->state,
	p->stack,
#ifdef CONFIG_VMAP_STACK
	p->stack_vm_area->addr,
#endif
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
	p->nr_cpus_allowed,
#ifdef CONFIG_SCHED_INFO
	p->sched_info.pcount,
	p->sched_info.run_delay // TODO :: unit ??
#endif
	);

	if (p->mm)
		pr_info("not a kernel thread; mm_struct  : 0x%lx\n", p->mm);
		// TODO : show mm details here
	else
		pr_info("is a kernel thread (mm NUL)\n");

	pr_info(
	" in execve()? %s\n"
	" in iowait  ? %s\n"
	" stack canary  : 0x%lx\n"
	" utime, stime  : %llu, %llu\n"
	" # vol c/s, # invol c/s : %6lu, %6lu\n"
	" # minor, major faults  : %6lu, %6lu\n"
	" login UID  : %d\n"
#ifdef CONFIG_LOCKDEP
	" lockdep depth : %d\n"
#endif
#ifdef CONFIG_TASK_IO_ACCOUNTING
	" task I/O accounting ::\n"
	"  read bytes               : %7llu\n"
	"  written (or will) bytes  : %7llu\n"
	"  cancelled write bytes    : %7llu\n"
#endif
#ifdef CONFIG_TASK_XACCT
	"  # read syscalls          : %7llu\n"
	"  # write syscalls         : %7llu\n"
	" accumulated RSS usage    : %9llu (%6llu KB)\n"  // bytes ??
	" accumulated  VM usage    : %9llu (%6llu KB)\n"  // bytes ??
#endif
#ifdef CONFIG_PSI
	" pressure stall state flags: 0x%lx\n"
#endif
	,
	p->in_execve == 1 ? "yes" : "no",
	p->in_iowait == 1 ? "yes" : "no",
	p->stack_canary,
	p->utime, p->stime,
	p->nvcsw, p->nivcsw,
	p->min_flt, p->maj_flt,
	__kuid_val(p->loginuid),
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
	p->acct_vm_mem1, p->acct_vm_mem1/1024,
#endif
#ifdef CONFIG_PSI
	p->psi_flags
#endif
	);

	/* thread struct : cpu-specific hardware context */
	pr_info(
	" h/w ctx info loc is thread_info : 0x%lx\n"
#ifdef CONFIG_X86_64
	"  sp  : 0x%lx\n"
	"  es  : 0x%x, ds  : 0x%x\n"
	"  cr2 : 0x%lx, trap #  : 0x%lx, error code  :  0x%lx\n"
	"  mm: addr limit (user boundary) : 0x%lx (%lu GB, %lu TB)\n" /* %lu EB)\n" */
#endif
#ifdef CONFIG_ARM64
	"  fault_address  : 0x%lx, fault code (ESR_EL1): 0x%lx\n"
#endif

	,
	&(p->thread),
#ifdef CONFIG_X86_64
	p->thread.sp,
	p->thread.es, p->thread.ds,
	p->thread.cr2, p->thread.trap_nr, p->thread.error_code,
	p->thread.addr_limit.seg, (unsigned long)p->thread.addr_limit.seg/(1024*1024),
	(unsigned long)p->thread.addr_limit.seg/(1024*1024*1024)
	/* (unsigned long)p->thread.addr_limit.seg/(1024*1024*1024*1024) : results in IoF */
#endif
#ifdef CONFIG_ARM64
	p->thread.fault_address, p->thread.fault_code
#endif
	);


#if 0
    dump_stack();
#endif
	task_unlock(p);
	return 0;
}

static int __init taskdtl_init(void)
{
	disp_task_details();
	return 0; // success
}
static void __exit taskdtl_exit(void)
{
	pr_info("exit: Process context :: %s PID %d\n", 
				current->comm, current->pid);
#if 0
    dump_stack();
#endif
}

module_init(taskdtl_init);
module_exit(taskdtl_exit);

