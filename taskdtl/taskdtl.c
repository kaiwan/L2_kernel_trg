#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>     // current
#include <linux/vmalloc.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
#include <linux/sched/signal.h>
#endif

MODULE_LICENSE("Dual MIT/GPL");

static int disp_task_details(struct task_struct *p)
{
	//struct task_struct *p = current;
	char *pol=NULL;

	task_lock(p);

	/* Task PID, ownership */
	pr_info("Process / Thread :: %s : TGID %d, PID %d\n"
		" RealUID : %d, EffUID : %d\n"
		" login UID  : %d\n",
		p->comm, p->tgid, p->pid,
		__kuid_val(p->cred->uid), __kuid_val(p->cred->euid),
		__kuid_val(p->loginuid));

	/* Task state */
	pr_info("Task state (%d) ::\n", p->state);
	switch (p->state) {
	case TASK_RUNNING : pr_info(" R : ready-to-run (on rq) OR running (on cpu)\n");
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
	default : pr_info(" <unknown>\n");
	}

	pr_info(
	"thread_info is "
#ifdef CONFIG_THREAD_INFO_IN_TASK
	"within the task struct itself\n"
#else
	"separate (not within the task struct)\n"
#endif
	"stack : 0x%lx"  /* we use the %lx fmt spec instead of the
			     * recommended-for-security %pK as we really do want
			     * to see the actual address here; don't do this in
			     * production! */
#ifdef CONFIG_VMAP_STACK
	"  (vmapped)\n"
#else
	"\n"
#endif
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	"  (GCC STACKLEAK) lowest stack : 0x%lx\n"
#endif
	"flags : 0x%x\n"
#ifdef CONFIG_THREAD_INFO_IN_TASK
	" curr CPU : %d\n"
#endif
	"sched ::\n"
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

	if (p->mm)
		pr_info("not a kernel thread; mm_struct   : 0x%lx\n", p->mm);
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
	" accumulated RSS usage     : %9llu (%6llu KB)\n"  // bytes ??
	" accumulated  VM usage     : %9llu (%6llu KB)\n"  // bytes ??
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
	" Hardware ctx info location is thread_info : 0x%lx\n"
#ifdef CONFIG_X86_64
	" X86_64 ::\n"
	"  sp  : 0x%lx\n"
	"  es  : 0x%x, ds  : 0x%x\n"
	"  cr2 : 0x%lx, trap #  : 0x%lx, error code  :  0x%lx\n"
	"  mm: addr limit (user boundary) : 0x%lx (%lu GB, %lu TB)\n" /* %lu EB)\n" */
#endif /* CONFIG_X86_64 */
#ifdef CONFIG_ARM64
	" ARM64 ::\n"
	" Saved registers ::\n"
	"  X19 = 0x%lx  X20 = 0x%lx   X21 = 0x%lx\n"
	"  X22 = 0x%lx  X23 = 0x%lx   X24 = 0x%lx\n"
	"  X25 = 0x%lx  X26 = 0x%lx   X27 = 0x%lx\n"
	"  X28 = 0x%lx\n"
	"   FP = 0x%lx   SP = 0x%lx    PC = 0x%lx\n"
	"  fault_address  : 0x%lx, fault code (ESR_EL1): 0x%lx\n"
	"\n arm64 pointer authentication"
 #ifdef CONFIG_ARM64_PTR_AUTH
	" present.\n"
 #else
	" absent.\n"
 #endif
#endif /* CONFIG_ARM64 */
	,
	&(p->thread),
#ifdef CONFIG_X86_64
	p->thread.sp,
	p->thread.es, p->thread.ds,
	p->thread.cr2, p->thread.trap_nr, p->thread.error_code,
	p->thread.addr_limit.seg, (unsigned long)p->thread.addr_limit.seg/(1024*1024),
	(unsigned long)p->thread.addr_limit.seg/(1024*1024*1024)
	/* (unsigned long)p->thread.addr_limit.seg/(1024*1024*1024*1024) : results in IoF ! */
#endif /* CONFIG_X86_64 */
#ifdef CONFIG_ARM64
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
	disp_task_details(current);
	return 0;	// success
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
