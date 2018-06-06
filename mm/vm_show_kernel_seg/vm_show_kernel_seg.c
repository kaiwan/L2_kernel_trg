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
	#define FMTSPC "%08x"
	#define FMTSPC_DEC "%08d"
	#define TYPECST unsigned int
    #define MY_PATTERN1   0xdeadface
    #define MY_PATTERN2   0xffeeddcc
#elif(BITS_PER_LONG == 64)
	#define FMTSPC "%016lx"
	#define FMTSPC_DEC "%ld"
	#define TYPECST unsigned long
    #define MY_PATTERN1   0xdeadfacedeadface
    #define MY_PATTERN2   0xffeeddccbbaa9988
#endif

static unsigned int statgul;
u32 guinit;
u32 ginit=0x5;
static int statint=0x1000;

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

#if(BITS_PER_LONG == 32)
	pr_info ("32-bit OS ");
#elif(BITS_PER_LONG == 64)
	pr_info ("64-bit OS ");
#endif
#ifdef __BIG_ENDIAN  // just for the heck of it..
	pr_info("Big-endian.\n");
#else
	pr_info("Little-endian.\n");
#endif

	/* Ha! When using "%d" etc for sizeof(), the compiler would complain:
	 * ... warning: format ‘%d’ expects argument of type ‘int’, but
	 * argument 5 has type ‘long unsigned int’ [-Wformat=] ...
	 * Turns out we shoud use "%zu" to correctly represent size_t (which sizeof operator returns)!
	 */
	pr_info ("sizeof(int)=%zu, sizeof(long)=%zu sizeof(void *)=%zu\nsizeof(u64 *)=%zu\n", 
		sizeof(int), sizeof(long), sizeof(void *), sizeof(u64 *));

	kptr = kmalloc (knum, GFP_KERNEL);
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
	print_hex_dump_bytes ("", DUMP_PREFIX_ADDRESS, kptr, disp);

	vptr = vmalloc (42*PAGE_SIZE);
	if (!vptr) {
		pr_alert("vmalloc failed!\n");
		kfree(kptr);
		return -ENOMEM;
	}
	mempattern(vptr, MY_PATTERN2, PAGE_SIZE);
	pr_info("vmalloc'ed memory dump (%d bytes @ %pK):\n", disp, vptr);
	print_hex_dump_bytes ("", DUMP_PREFIX_ADDRESS, vptr, disp);

	pr_info (
    "\nSome Kernel Details [sorted by decreasing address] -------------------\n"
	"FIXADDR_START       = 0x" FMTSPC "\n"
	"MODULES_END         = 0x" FMTSPC "\n"
	"MODULES_VADDR       = 0x" FMTSPC " [modules range: " FMTSPC_DEC " MB]\n"
	"CPU_ENTRY_AREA_BASE = 0x" FMTSPC "\n"
	"VMEMMAP_START       = 0x" FMTSPC "\n"
	"VMALLOC_END         = 0x" FMTSPC "\n"
	"VMALLOC_START       = 0x" FMTSPC " [vmalloc range: " FMTSPC_DEC " MB =" FMTSPC_DEC " GB]" "\n"
	"PAGE_OFFSET         = 0x" FMTSPC " [start of all phy mapped RAM; lowmem]\n"
    "TASK_SIZE           = 0x" FMTSPC " [size of userland]\n",
		(TYPECST)FIXADDR_START,
		(TYPECST)MODULES_END, (TYPECST)MODULES_VADDR,
		 (TYPECST)((MODULES_END-MODULES_VADDR)/(1024*1024)),
		(TYPECST)CPU_ENTRY_AREA_BASE,
		(TYPECST)VMEMMAP_START,
		(TYPECST)VMALLOC_END, (TYPECST)VMALLOC_START,
		 (TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024)), 
         (TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024*1024)),
		(TYPECST)PAGE_OFFSET,
		(TYPECST)TASK_SIZE);

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
	" Statistics wrt 'current' [process/thread TGID=%d PID=%d name=%s]:\n"
	"arg_end     = 0x" FMTSPC "\n"
	"arg_start   = 0x" FMTSPC "\n"
	"start_stack = 0x" FMTSPC "\n"
	"curr brk    = 0x" FMTSPC "\n" 
	"start_brk   = 0x" FMTSPC "\n"
	"env_end     = 0x" FMTSPC "\n"
	"env_start   = 0x" FMTSPC "\n"
	"end_data    = 0x" FMTSPC "\n"
	"start_data  = 0x" FMTSPC "\n" 
	"end_code    = 0x" FMTSPC "\n"
	"start_code  = 0x" FMTSPC "\n"
	"# memory regions (VMAs) = %d\n",
		current->tgid, current->pid, current->comm,
		(TYPECST)current->mm->arg_end, 
		(TYPECST)current->mm->arg_start,
		(TYPECST)current->mm->start_stack,
		(TYPECST)current->mm->brk,
		(TYPECST)current->mm->start_brk,
		(TYPECST)current->mm->env_end,
		(TYPECST)current->mm->env_start,
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
dmesg output of vm_img_lkm.c on a 32-bit Linux system
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


===============================================================================
dmesg output of vm_img_lkm.c on a 64-bit Linux system
 (with indentation to make it human-readable)
===============================================================================
[432111.443835] vm_img_lkm: module license 'GPL/LGPL' taints kernel.
[432111.443838] Disabling lock debugging due to kernel taint
[432111.444208] vm_img_init:69 : 64-bit OS Little-endian.
[432111.444211] vm_img_init:77 : sizeof(int) = 4, sizeof(long) = 8 sizeof(void *)=8

[432111.444212] vm_img_init:85 : kmalloc'ed memory dump:
[432111.444214] ffff8800966f6ca0: ce fa ad de ce fa ad de ce fa ad de ce fa ad de  ................
[432111.444215] ffff8800966f6cb0: 6b 6d 00 00 00 00 00 00 00 00 00 00 00 00 00 00  km..............

[432111.444225] vm_img_init:95 : vmalloc'ed memory start: 0xffffc90005a11000

[432111.444228] vm_img_init:118 : Statistics wrt 'current'  [which right now is the process/thread TGID 19903 PID 19903 name insmod]:
[432111.444228] PAGE_OFFSET = 0xffff880000000000 TASK_SIZE=0x7ffffffff000
[432111.444228] VMALLOC_START = 0xffffc90000000000 VMALLOC_END=0xffffe8ffffffffff : vmalloc range: 0x01ffffff MB
[432111.444228] MODULES_VADDR = 0xffffffffa0000000 MODULES_END=0xffffffffff000000 : modules range: 0x000005f0 MB
[432111.444228] start_code = 0x7fb85339a000, end_code = 0x7fb8533bd2dc, start_data = 0x7fb8535be0d8, end_data = 0x7fb8535bf3a0
[432111.444228] start_brk  = 0x7fb8553f6000, brk = 0x7fb855417000, start_stack = 0x7fff3e9ed550
[432111.444228] arg_start  = 0x7fff3e9ed7e8, arg_end = 0x7fff3e9ed7ff, env_start = 0x7fff3e9ed7ff, env_end = 0x7fff3e9edfeb

[432111.444228] eg. kernel vaddr: &statgul = 0xffffffffa06103b8, &jiffies_64 = 0xffffffff81d1d000, &vg = 0xffffffffa0610100
[432111.444228] kptr = 0xffff8800966f6ca0 vptr = 0xffffc90005a11000
[432111.444232] foo:46 : &c = 0xffff8800966cdc2f

[432111.444232] bar:39 : ---------------------Stack Dump:-------------------------------
[432111.444235] CPU: 1 PID: 19903 Comm: insmod Tainted: PF          O 3.13.0-24-generic #46-Ubuntu
[432111.444236] Hardware name:                  /DB85FL, BIOS FLB8510H.86A.0019.2013.0606.1538 06/06/2013
[432111.444237]  00007fff3e9ed7ff ffff8800966cdc08 ffffffff81715a64 ffffc90000000000
[432111.444239]  ffff8800966cdc18 ffffffffa060e025 ffff8800966cdc30 ffffffffa060e0ad
[432111.444241]  78ff88020dfbcc78 ffff8800966cdd40 ffffffffa021c3c0 ffff880000000000
[432111.444243] Call Trace:
[432111.444249]  [<ffffffff81715a64>] dump_stack+0x45/0x56
[432111.444252]  [<ffffffffa060e025>] bar+0x25/0x80 [vm_img_lkm]
[432111.444254]  [<ffffffffa060e0ad>] foo+0x2d/0x51 [vm_img_lkm]
[432111.444256]  [<ffffffffa021c3c0>] vm_img_init+0x3c0/0x1000 [vm_img_lkm]
[432111.444268]  [<ffffffffa0000000>] ? 0xffffffff9fffffff
[432111.444271]  [<ffffffffa021c000>] ? 0xffffffffa021bfff
[432111.444275]  [<ffffffff8100214a>] do_one_initcall+0xfa/0x1b0
[432111.444277]  [<ffffffff810598d3>] ? set_memory_nx+0x43/0x50
[432111.444281]  [<ffffffff810e1d4d>] load_module+0x12dd/0x1b40
[432111.444285]  [<ffffffff810dd7d0>] ? store_uevent+0x40/0x40
[432111.444287]  [<ffffffff810e2726>] SyS_finit_module+0x86/0xb0
[432111.444290]  [<ffffffff8172663f>] tracesys+0xe1/0xe6
[432111.444291] bar:41 : ---------------------------------------------------------------

===============================================================================

 */
