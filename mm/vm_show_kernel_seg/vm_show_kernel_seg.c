/*
 * vm_show_kernel_seg.c
 *
 * Simple kernel module to show various kernel virtual addresses, including
 * some user process context virtual addresses.
 * Useful for learning, testing, etc.
 * For both 32 and 64-bit systems.
 *
 * (c) 2011-2018 Kaiwan NB, kaiwanTECH.
 * License: MIT
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include "convenient.h"  // adjust path as required

#if(BITS_PER_LONG == 32)
	#define FMTSPC		"%08x"
	#define FMTSPC_DEC	"%08d"
	#define TYPECST		unsigned int
	#define MY_PATTERN1     0xdeadface
	#define MY_PATTERN2     0xffeeddcc
#elif(BITS_PER_LONG == 64)
	#define FMTSPC		"%016lx"
	#define FMTSPC_DEC	"%ld"
	#define TYPECST	        unsigned long
	#define MY_PATTERN1     0xdeadfacedeadface
	#define MY_PATTERN2     0xffeeddccbbaa9988
#endif

static unsigned int statgul;

void bar(void)
{
	pr_info("---------------------Stack Dump:-------------------------------\n");
	dump_stack();
	pr_info("---------------------------------------------------------------\n");
}
void foo(void)
{
	char c='x';
	pr_info("&c = 0x" FMTSPC "\n", (TYPECST)&c);
	bar();
}

// Init mem to a pattern (like 0xdead 0xface .... ....)
static inline void mempattern(
 void *pdest, 
#if(BITS_PER_LONG == 32)
 u32 pattern, 
#elif(BITS_PER_LONG == 64)
 u64 pattern, 
#endif
 int len)
{
	int i;
#if(BITS_PER_LONG == 32)
	u32 *pattptr = (u32 *)pdest;
	for (i=0; i < len/sizeof(u32 *); i++)
#elif(BITS_PER_LONG == 64)
	u64 *pattptr = (u64 *)pdest;
	for (i=0; i < len/sizeof(u64 *); i++)
#endif
                          {
		//pr_info("pattptr=%pK\n", pattptr);
		*pattptr = pattern;
		pattptr ++;
	}
}

static void *kptr=NULL, *vptr=NULL;
volatile static	u32 vg=0x1234abcd;

static int __init vm_img_init(void)
{
   int knum=512,disp=32;

   pr_info("Platform:\n");
#ifdef CONFIG_X86
 #if(BITS_PER_LONG == 32)
	pr_info(" x86-32 : ");
 #else
	pr_info(" x86_64 : ");
 #endif
#endif
#ifdef CONFIG_ARM
	pr_info(" ARM-32 : ");
#endif
#ifdef CONFIG_ARM64
	pr_info(" ARM64 : ");
#endif
#ifdef CONFIG_MIPS
	pr_info(" MIPS : ");
#endif
#ifdef CONFIG_PPC
	pr_info(" PPC : ");
#endif

#if(BITS_PER_LONG == 32)
	pr_info (" 32-bit OS ");
#elif(BITS_PER_LONG == 64)
	pr_info (" 64-bit OS ");
#endif
#ifdef __BIG_ENDIAN  // just for the heck of it..
	pr_info(" Big-endian.\n");
#else
	pr_info(" Little-endian.\n");
#endif

	/* Ha! When using "%d" etc for sizeof(), the compiler would complain:
	 * ... warning: format ‘%d’ expects argument of type ‘int’, but
	 * argument 5 has type ‘long unsigned int’ [-Wformat=] ...
	 * Turns out we shoud use "%zu" to correctly represent size_t (which sizeof operator returns)!
	 */
	pr_info (" sizeof(int)   =%zu, sizeof(long) =%zu\n"
		 " sizeof(void *)=%zu, sizeof(u64 *)=%zu\n", 
		sizeof(int), sizeof(long), sizeof(void *), sizeof(u64 *));

	kptr = kmalloc(knum, GFP_KERNEL);
	if (!kptr) {
		pr_alert("kmalloc failed!\n");
		return -ENOMEM;
	}
	/*
	 * !IMP! NOTE reg the 'new' security-conscious printk formats!
	 * SHORT story:
	 * We need to avoid 'leaking' kernel addr to userspace (hackers
	 * have a merry time!). 
	 * Now %p will _not_ show the actual kernel addr, but rather a 
	 * hashed value.
	 * 
	 * To see the actual addr use %px : dangerous!
	 * To see the actual addr iff root, use %pK : tunable via
	 *   /proc/sys/kernel/kptr_restrict : 
	 *              = 0 : hashed addr [default]
	 *              = 1 : _and_ root, actual addr displayed!
	 *              = 2 : never displayed.
	 *
	 * DETAILS:
	 * See https://www.kernel.org/doc/Documentation/printk-formats.txt
	 */
	pr_info("kmalloc'ed memory dump (%d bytes @ %pK):\n", disp, kptr);
	mempattern(kptr, MY_PATTERN1, knum);
	print_hex_dump_bytes("", DUMP_PREFIX_ADDRESS, kptr, disp);

	vptr = vmalloc(42*PAGE_SIZE);
	if (!vptr) {
		pr_alert("vmalloc failed!\n");
		kfree(kptr);
		return -ENOMEM;
	}
	mempattern(vptr, MY_PATTERN2, PAGE_SIZE);
	pr_info("vmalloc'ed memory dump (%d bytes @ %pK):\n", disp, vptr);
	print_hex_dump_bytes("", DUMP_PREFIX_ADDRESS, vptr, disp);

	pr_info (
    "\nSome Kernel Details [sorted by decreasing address] -------------------\n"
#ifdef CONFIG_X86
	" FIXADDR_START       = 0x" FMTSPC "\n"
#endif
	" MODULES_END         = 0x" FMTSPC "\n"
	" MODULES_VADDR       = 0x" FMTSPC " [modules range: " FMTSPC_DEC " MB]\n"
#ifdef CONFIG_X86
	" CPU_ENTRY_AREA_BASE = 0x" FMTSPC "\n"
	" VMEMMAP_START       = 0x" FMTSPC "\n"
#endif
	" VMALLOC_END         = 0x" FMTSPC "\n"
	" VMALLOC_START       = 0x" FMTSPC " [vmalloc range: " FMTSPC_DEC " MB =" FMTSPC_DEC " GB]" "\n"
	" PAGE_OFFSET         = 0x" FMTSPC " [lowmem region: start of all phy mapped RAM (here to RAM-size)]\n",
#ifdef CONFIG_X86
		(TYPECST)FIXADDR_START,
#endif
		(TYPECST)MODULES_END, (TYPECST)MODULES_VADDR,
		 (TYPECST)((MODULES_END-MODULES_VADDR)/(1024*1024)),
#ifdef CONFIG_X86
		(TYPECST)CPU_ENTRY_AREA_BASE,
		(TYPECST)VMEMMAP_START,
#endif
		(TYPECST)VMALLOC_END, (TYPECST)VMALLOC_START,
		 (TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024)), 
		 (TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024*1024)),
		(TYPECST)PAGE_OFFSET);

#ifdef CONFIG_KASAN
	pr_info("\nKASAN_SHADOW_START = 0x" FMTSPC " KASAN_SHADOW_END = 0x" FMTSPC "\n",
		(TYPECST)KASAN_SHADOW_START, (TYPECST)KASAN_SHADOW_END);
#endif
	/* 
	 * arch/x86/mm/dump_pagetables.c:address_markers[] array of structs would
	 * be useful to dump, but it's private
	pr_info("address_markers = 0x" FMTSPC FMTSPC "\n", address_markers);
	 */

	pr_info (
    "\nSome Process Details [sorted by decreasing address] ------------------\n"
	" [TASK_SIZE         = 0x" FMTSPC " size of userland]\n"
	" [Statistics wrt 'current' thread TGID=%d PID=%d name=%s]:\n"
	"        env_end     = 0x" FMTSPC "\n"
	"        env_start   = 0x" FMTSPC "\n"
	"        arg_end     = 0x" FMTSPC "\n"
	"        arg_start   = 0x" FMTSPC "\n"
	"        start_stack = 0x" FMTSPC "\n"
	"        curr brk    = 0x" FMTSPC "\n" 
	"        start_brk   = 0x" FMTSPC "\n"
	"        end_data    = 0x" FMTSPC "\n"
	"        start_data  = 0x" FMTSPC "\n" 
	"        end_code    = 0x" FMTSPC "\n"
	"        start_code  = 0x" FMTSPC "\n"
	" [# memory regions (VMAs) = %d]\n",
		(TYPECST)TASK_SIZE,
		current->tgid, current->pid, current->comm,
		(TYPECST)current->mm->env_end,
		(TYPECST)current->mm->env_start,
		(TYPECST)current->mm->arg_end, 
		(TYPECST)current->mm->arg_start,
		(TYPECST)current->mm->start_stack,
		(TYPECST)current->mm->brk,
		(TYPECST)current->mm->start_brk,
		(TYPECST)current->mm->end_data,
		(TYPECST)current->mm->start_data,
		(TYPECST)current->mm->end_code,
		(TYPECST)current->mm->start_code,
		current->mm->map_count);

	pr_info (
	"\nSome sample kernel virtual addreses ---------------------\n" 
	"&statgul = 0x" FMTSPC ", &jiffies_64 = 0x%08lx, &vg = 0x" FMTSPC "\n"
	"kptr = 0x" FMTSPC " vptr = 0x" FMTSPC "\n",
		(TYPECST)&statgul, (long unsigned int)&jiffies_64, (TYPECST)&vg,
		(TYPECST)kptr, (TYPECST)vptr);

	foo();
	return 0;
}

static void __exit vm_img_exit(void)
{
	vfree (vptr);
	kfree (kptr);
	pr_info("Done.\n");
}

module_init(vm_img_init);
module_exit(vm_img_exit);

MODULE_AUTHOR("Kaiwan N Billimoria <kaiwan at kaiwantech dot com>");
MODULE_LICENSE("Dual GPL/MIT");

/*
 * -------------------------- Sample OUTPUT ----------------------------------
===============================================================================
dmesg output of vm_show_kernel_seg.c on an x86_64 Linux system (Fedora 28)
===============================================================================

[85850.257391] Platform:
[85850.257395]  x86_64 : 
[85850.257395]  64-bit OS 
[85850.257397]  Little-endian.
[85850.257399]  sizeof(int)   =4, sizeof(long) =8
                sizeof(void *)=8, sizeof(u64 *)=8
[85850.257403] kmalloc'ed memory dump (32 bytes @ ffff90a789e13600):
[85850.257406] 0000000083c03d41: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................
[85850.257407] 000000008154b8a3: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................
[85850.257846] vmalloc'ed memory dump (32 bytes @ ffff9c540bc51000):
[85850.257849] 0000000097b70711: 88 99 aa bb cc dd ee ff 88 99 aa bb cc dd ee ff  ................
[85850.257850] 0000000079d92e44: 88 99 aa bb cc dd ee ff 88 99 aa bb cc dd ee ff  ................
[85850.257852] 
               Some Kernel Details [sorted by decreasing address] -------------------
                FIXADDR_START       = 0xffffffffff576000
                MODULES_END         = 0xffffffffff000000
                MODULES_VADDR       = 0xffffffffc0000000 [modules range: 1008 MB]
                CPU_ENTRY_AREA_BASE = 0xfffffe0000000000
                VMEMMAP_START       = 0xffffeccec0000000
                VMALLOC_END         = 0xffffbc53ffffffff
                VMALLOC_START       = 0xffff9c5400000000 [vmalloc range: 33554431 MB =32767 GB]
                PAGE_OFFSET         = 0xffff90a3c0000000 [lowmem region: start of all phy mapped RAM (here to RAM-size)]
[85850.257860] 
               Some Process Details [sorted by decreasing address] ------------------
                [TASK_SIZE         = 0x00007ffffffff000 size of userland]
                [Statistics wrt 'current' thread TGID=20577 PID=20577 name=insmod]:
                       env_end     = 0x00007ffeb9904fe7
                       env_start   = 0x00007ffeb9904780
                       arg_end     = 0x00007ffeb9904780
                       arg_start   = 0x00007ffeb9904761
                       start_stack = 0x00007ffeb9903290
                       curr brk    = 0x000055a288079000
                       start_brk   = 0x000055a288058000
                       end_data    = 0x000055a287e63080
                       start_data  = 0x000055a287e61cf0
                       end_code    = 0x000055a287c610e7
                       start_code  = 0x000055a287c3e000
                [# memory regions (VMAs) = 35]
[85850.257867] 
               Some sample kernel virtual addreses ---------------------
               &statgul = 0xffffffffc15f7410, &jiffies_64 = 0xffffffff88205000, &vg = 0xffffffffc15f7030
               kptr = 0xffff90a789e13600 vptr = 0xffff9c540bc51000
[85850.257870] &c = 0xffff9c540cf37c3f
[85850.257871] ---------------------Stack Dump:-------------------------------
[85850.257873] CPU: 2 PID: 20577 Comm: insmod Tainted: P           OE    4.16.13-300.fc28.x86_64 #1
[85850.257874] Hardware name: LENOVO 20FMA089IG/20FMA089IG, BIOS R06ET39W (1.13 ) 07/11/2016
[85850.257876] Call Trace:
[85850.257882]  dump_stack+0x5c/0x85
[85850.257887]  bar+0x16/0x22 [vm_show_kernel_seg]
[85850.257890]  foo+0x34/0x4e [vm_show_kernel_seg]
[85850.257893]  vm_img_init+0x30c/0x1000 [vm_show_kernel_seg]
[85850.257895]  ? 0xffffffffc133c000
[85850.257898]  do_one_initcall+0x48/0x13b
[85850.257902]  ? free_unref_page_commit+0x9b/0x110
[85850.257904]  ? _cond_resched+0x15/0x30
[85850.257907]  ? kmem_cache_alloc_trace+0x111/0x1c0
[85850.257910]  ? do_init_module+0x22/0x210
[85850.257912]  ? do_init_module+0x5a/0x210
[85850.257914]  ? load_module+0x210f/0x24a0
[85850.257917]  ? vfs_read+0x110/0x140
[85850.257920]  ? SYSC_finit_module+0xad/0x110
[85850.257922]  ? SYSC_finit_module+0xad/0x110
[85850.257925]  ? do_syscall_64+0x74/0x180
[85850.257927]  ? entry_SYSCALL_64_after_hwframe+0x3d/0xa2
[85850.257929] ---------------------------------------------------------------

=======================================================================================
dmesg output of vm_show_kernel_seg.c on an ARM-32 Linux system (Qemu-emulated Vexpress)
=======================================================================================
vm_show_kernel_seg: loading out-of-tree module taints kernel.
vm_show_kernel_seg: module license 'Dual GPL/MIT' taints kernel.
Disabling lock debugging due to kernel taint
Platform:
 ARM-32 : 
 32-bit OS 
 Little-endian.
 sizeof(int)   =4, sizeof(long) =4
 sizeof(void *)=4, sizeof(u64 *)=4
kmalloc'ed memory dump (32 bytes @ 9ee94200):
9ee94200: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................
9ee94210: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................
vmalloc'ed memory dump (32 bytes @ a5346000):
a5346000: cc dd ee ff cc dd ee ff cc dd ee ff cc dd ee ff  ................
a5346010: cc dd ee ff cc dd ee ff cc dd ee ff cc dd ee ff  ................

Some Kernel Details [sorted by decreasing address] -------------------
 MODULES_END         = 0x80000000
 MODULES_VADDR       = 0x7f000000 [modules range: 00000016 MB]
 VMALLOC_END         = 0xff800000
 VMALLOC_START       = 0xa0800000 [vmalloc range: 00001520 MB =00000001 GB]
 PAGE_OFFSET         = 0x80000000 [lowmem region: start of all phy mapped RAM (here to RAM-size)]

Some Process Details [sorted by decreasing address] ------------------
 [TASK_SIZE         = 0x7f000000 size of userland]
 [Statistics wrt 'current' thread TGID=736 PID=736 name=insmod]:
        env_end     = 0x7e992fef
        env_start   = 0x7e992f36
        arg_end     = 0x7e992f36
        arg_start   = 0x7e992f19
        start_stack = 0x7e992e20
        curr brk    = 0x000e4000
        start_brk   = 0x000c3000
        end_data    = 0x000c2719
        start_data  = 0x000c1f08
        end_code    = 0x000b1590
        start_code  = 0x00010000
 [# memory regions (VMAs) = 21]

Some sample kernel virtual addreses ---------------------
&statgul = 0x7f000908, &jiffies_64 = 0x80a02d00, &vg = 0x7f0006e4
kptr = 0x9ee94200 vptr = 0xa5346000
&c = 0x9ee89d87
---------------------Stack Dump:-------------------------------
CPU: 0 PID: 736 Comm: insmod Tainted: P           O    4.9.1 #4
Hardware name: ARM-Versatile Express
[<80110e98>] (unwind_backtrace) from [<8010cbc0>] (show_stack+0x20/0x24)
[<8010cbc0>] (show_stack) from [<803f5714>] (dump_stack+0xb0/0xdc)
[<803f5714>] (dump_stack) from [<7f000024>] (bar+0x24/0x34 [vm_show_kernel_seg])
[<7f000024>] (bar [vm_show_kernel_seg]) from [<7f002298>] (vm_img_init+0x298/0x2a8 [vm_show_kernel_seg])
[<7f002298>] (vm_img_init [vm_show_kernel_seg]) from [<80101d34>] (do_one_initcall+0x64/0x1ac)
[<80101d34>] (do_one_initcall) from [<801ff884>] (do_init_module+0x74/0x1e4)
[<801ff884>] (do_init_module) from [<8019f188>] (load_module+0x1cc4/0x22f8)
[<8019f188>] (load_module) from [<8019f918>] (SyS_init_module+0x15c/0x17c)
[<8019f918>] (SyS_init_module) from [<80108220>] (ret_fast_syscall+0x0/0x1c)
---------------------------------------------------------------

===============================================================================
[OLDER] dmesg output of vm_img_lkm.c on an IA-32 Linux system
 (with indentation to make it human-readable)
===============================================================================
[79146.354770] vm_img_init:67 : 32-bit OS Little-endian.
[79146.354775] vm_img_init:77 : sizeof(int) = 4, sizeof(long) = 4 sizeof(void *)=4

[79146.354777] vm_img_init:85 : kmalloc'ed memory dump:
[79146.354780] efeae780: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................
[79146.354782] efeae790: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................

[79146.354795] vm_img_init:95 : vmalloc'ed memory start: 0xf95f6000

[79146.354802] vm_img_init:118 : Statistics wrt 'current'  [which right now is the process/thread TGID 26512 PID 26512 name insmod]:
[79146.354802] PAGE_OFFSET = 0xc0000000 TASK_SIZE=0xc0000000
[79146.354802] VMALLOC_START = 0xf83fe000 VMALLOC_END=0xffbfe000 : vmalloc range: 0x00000078 MB
[79146.354802] MODULES_VADDR = 0xf83fe000 MODULES_END=0xffbfe000 : modules range: 0x00000078 MB
[79146.354802] start_code = 0xb7780000, end_code = 0xb77a56b0, start_data = 0xb77a6850, end_data = 0xb77a71c0
[79146.354802] start_brk  = 0xb8e3f000, brk = 0xb8e60000, start_stack = 0xbf871260
[79146.354802] arg_start  = 0xbf8717aa, arg_end = 0xbf8717c1, env_start = 0xbf8717c1, env_end = 0xbf871fef

[79146.354802] eg. kernel vaddr: &statgul = 0xfabf226c, &jiffies_64 = 0xc191e240, &vg = 0xfabf20c0
[79146.354802] kptr = 0xefeae780 vptr = 0xf95f6000
[79146.354809] foo:46 : &c = 0xf197fd6f

[79146.354810] bar:39 : ---------------------Stack Dump:-------------------------------
[79146.354813] CPU: 2 PID: 26512 Comm: insmod Tainted: P           OX 3.13.0-37-generic #64-Ubuntu
[79146.354815] Hardware name: LENOVO 4291GG9/4291GG9, BIOS 8DET42WW (1.12 ) 04/01/2011
[79146.354816]  00000000 00000000 f197fd40 c1653867 ffbfe000 f197fd54 fabf0023 fabf1024
[79146.354822]  fabf1499 00000027 f197fd70 fabf00a7 fabf140f fabf1495 0000002e f197fd6f
[79146.354827]  7897fd7c f197fdfc fabf5348 fabf1150 fabf1489 00000076 00006790 00006790
[79146.354833] Call Trace:
[79146.354841]  [<c1653867>] dump_stack+0x41/0x52
[79146.354845]  [<fabf0023>] bar+0x23/0x80 [vm_img_lkm]
[79146.354848]  [<fabf00a7>] foo+0x27/0x4e [vm_img_lkm]
[79146.354851]  [<fabf5348>] vm_img_init+0x348/0x1000 [vm_img_lkm]
[79146.354859]  [<fabf5000>] ? 0xfabf4fff
[79146.354864]  [<c1002122>] do_one_initcall+0xd2/0x190
[79146.354867]  [<fabf5000>] ? 0xfabf4fff
[79146.354873]  [<c104c91f>] ? set_memory_nx+0x5f/0x70
[79146.354876]  [<c164f5f1>] ? set_section_ro_nx+0x54/0x59
[79146.354880]  [<c10c4371>] load_module+0x1121/0x18e0
[79146.354887]  [<c10c4c95>] SyS_finit_module+0x75/0xc0
[79146.354891]  [<c113a02b>] ? vm_mmap_pgoff+0x7b/0xa0
[79146.354901]  [<c1661bcd>] sysenter_do_call+0x12/0x12
[79146.354903] bar:41 : ---------------------------------------------------------------

 */
