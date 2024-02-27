/*
 * oom-killer-try.c
 * Demo the kernel OOM killer by having this process call malloc()
 * repetedly without freeing.
 * Ultimately, the kernel will kill it via OOM.
 * For a more "sure" kill, set the "force-page-fault" flag.
 *
 ** WARNING **
 * Be warned that running this intensively can/will cause 
 * heavy swapping on your system and will probably necessitate a
 * reboot.
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * License: MIT
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BLK	(getpagesize()*2)

int force_page_fault = 0;

/*
 * This code (the cat,awk,.. commands) will actually stop working when memory 
 * pressure becomes high - as userspace app requests for memory will be ignored
 * by the kernel
 */
static void see_maps(void)
{
#if 0
	int s;
	char cmd[128];

	sprintf(cmd, "cat /proc/%d/maps |awk '$6==\"[heap]\" {print $0}'",
		getpid());
	s = system(cmd);
	if (s == -1) {
		perror("system");// when memory runs short, we get: "system: Cannot allocate memory" msg..
	}
	fflush(stdout);
#endif
}

int main(int argc, char **argv)
{
	char *p;
	int i = 0, stepval = 5000, verbose = 0;

	if (argc < 3) {
		fprintf(stderr,
			"Usage: %s alloc-loop-count force-page-fault[0|1] [verbose_flag[0|1]]\n",
			argv[0]);
		exit(1);
	}

	printf("%s: PID %d\n", argv[0], getpid());
	if (atoi(argv[2]) == 1)
		force_page_fault = 1;

	if (argc >= 4) {
		if (atoi(argv[3]) == 1)
			verbose = 1;
	}

	do {
		p = (char *)malloc(BLK);
		if (verbose && !(i % stepval)) 	// every 'stepval' iterations..
			printf("%06d\taddr p = %p   break = %p\n",
				i, (void *)p, (void *)sbrk(0));
		if (!p) {
			fprintf(stderr, "%s: loop #%d: malloc failure.\n",
				argv[0], i);
			break;
		}

		/* Force the MMU to raise page fault exception by writing into the page;
		 * writing a single byte, any byte, will do the trick!
		 * This is as the virtual address referenced will have no PTE entry, causing the MMU
		 * to raise the page fault!
		 * The fault handler, being intelligent, figures out it's a "good fault" and allocates a 
		 * page frame. Only now do we have physical memory!
		 */
		if (force_page_fault) {
			p[4000] &= 0x14;
			p[8000] |= 'a';
		}

/*
		if (!(i % stepval)) {	// every 'stepval' iterations..
			see_maps();
			//usleep(250000);       // 250ms; to have time for sampling free mem..
		} */
		i++;
	} while (p && (i < atoi(argv[1])));

	see_maps();
	return 0;
}


/***************************************************
-------------- Sample Runs: ----------------------
On a ARM/Linux VM with 256 MB RAM emulated using QEMU:

ARM /myprj $ uname -a
Linux (none) 3.2.21 #21 SMP Thu Jan 10 12:48:07 IST 2013 armv7l GNU/Linux
ARM /myprj $

ARM /myprj $ ./oom-killer-try 70000 1
./oom-killer-try: PID 474
0...
00012000-00035000 rw-p 00000000 00:00 0          [heap]
5000...
00012000-0273c000 rw-p 00000000 00:00 0          [heap]
10000...
00012000-04e64000 rw-p 00000000 00:00 0          [heap]
15000...
00012000-07568000 rw-p 00000000 00:00 0          [heap]
20000...
system: Cannot allocate memory
25000...
system: Cannot allocate memory
[   15.437810] oom-killer-try invoked oom-killer: gfp_mask=0x200da, order=0, oom_adj=0, oom_score_adj=0
[   15.438675] oom-killer-try cpuset=/ mems_allowed=0
[   15.439804] [<c0017674>] (unwind_backtrace+0x0/0x108) from [<c041886c>] (dump_stack+0x20/0x24)
[   15.440496] [<c041886c>] (dump_stack+0x20/0x24) from [<c00cd1fc>] (T.390+0xa4/0x200)
[   15.441116] [<c00cd1fc>] (T.390+0xa4/0x200) from [<c00cd3d4>] (T.388+0x7c/0x2b8)
[   15.441713] [<c00cd3d4>] (T.388+0x7c/0x2b8) from [<c00cd894>] (out_of_memory+0x284/0x400)
[   15.442347] [<c00cd894>] (out_of_memory+0x284/0x400) from [<c00d3354>] (__alloc_pages_nodemask+0x720/0x734)
[   15.443105] [<c00d3354>] (__alloc_pages_nodemask+0x720/0x734) from [<c00ee3ac>] (do_wp_page+0x43c/0x934)
[   15.443817] [<c00ee3ac>] (do_wp_page+0x43c/0x934) from [<c00eeb98>] (handle_pte_fault+0x2f4/0x870)
[   15.444544] [<c00eeb98>] (handle_pte_fault+0x2f4/0x870) from [<c00ef6c4>] (handle_mm_fault+0xd4/0x19c)
[   15.445253] [<c00ef6c4>] (handle_mm_fault+0xd4/0x19c) from [<c041e2ac>] (do_page_fault+0x284/0x3c8)
[   15.445913] [<c041e2ac>] (do_page_fault+0x284/0x3c8) from [<c00084b8>] (do_DataAbort+0x48/0xac)
[   15.446581] [<c00084b8>] (do_DataAbort+0x48/0xac) from [<c041c69c>] (__dabt_usr+0x3c/0x40)
[   15.447357] Exception stack(0xce1f9fb0 to 0xce1f9ff8)
[   15.447795] 9fa0:                                     0dbf4eb8 4023c238 0dbf5e58 00000000
[   15.448344] 9fc0: be818f63 00008e30 00000000 00000000 00000000 00000000 4023cb60 be818d2c
[   15.448895] 9fe0: 0000a149 be818d08 4016cbac 00008978 20000010 ffffffff
[   15.449446] Mem-info:
[   15.449705] Normal per-cpu:
[   15.449953] CPU    0: hi:   90, btch:  15 usd:   0
[   15.450400] active_anon:56341 inactive_anon:0 isolated_anon:0
[   15.450412]  active_file:0 inactive_file:0 isolated_file:0
[   15.450423]  unevictable:5240 dirty:0 writeback:0 unstable:0
[   15.450434]  free:511 slab_reclaimable:216 slab_unreclaimable:296
[   15.450446]  mapped:203 shmem:0 pagetables:118 bounce:0
[   15.452669] Normal free:2044kB min:2036kB low:2544kB high:3052kB active_anon:225364kB inactive_anon:0kB active_file:0kB inactive_file:0kB unevictable:20960kB isolated(anon):0kB isolated(file):0kB present:260096kB mlocked:0kB dirty:0kB writeback:0kB mapped:812kB shmem:0kB slab_reclaimable:864kB slab_unreclaimable:1184kB kernel_stack:192kB pagetables:472kB unstable:0kB bounce:0kB writeback_tmp:0kB pages_scanned:0 all_unreclaimable? yes
[   15.455411] lowmem_reserve[]: 0 0
[   15.455820] Normal: 3*4kB 0*8kB 1*16kB 1*32kB 1*64kB 1*128kB 1*256kB 1*512kB 1*1024kB 0*2048kB 0*4096kB = 2044kB
[   15.457006] 5242 total pagecache pages
[   15.457326] 0 pages in swap cache
[   15.457599] Swap cache stats: add 0, delete 0, find 0/0
[   15.458018] Free swap  = 0kB
[   15.458252] Total swap = 0kB
[   15.461030] 65536 pages of RAM
[   15.461321] 547 free pages
[   15.461544] 2170 reserved pages
[   15.461796] 446 slab pages
[   15.462020] 428 pages shared
[   15.462269] 0 pages swap cached
[   15.462542] [ pid ]   uid  tgid total_vm      rss cpu oom_adj oom_score_adj name
[   15.463174] [  470]     0   470      820      163   0       0             0 sh
[   15.463750] [  474]     0   474    56723    56384   0       0             0 oom-killer-try
[   15.464366] Out of memory: Kill process 474 (oom-killer-try) score 861 or sacrifice child
[   15.464929] Killed process 474 (oom-killer-try) total-vm:226892kB, anon-rss:225060kB, file-rss:476kB
Killed
ARM /myprj $ 

-----------------------------------------------------------------------
On a PC running SLES 10:

$ cat /etc/issue

Welcome to SUSE Linux Enterprise Server 10 (i586) - Kernel \r (\l).

$
$ cat /proc/version
Linux version 2.6.16.21-0.8-smp (geeko@buildhost) (gcc version 4.1.0 (SUSE Linux)) #1 SMP Mon Jul 3 18:25:39 UTC 2006

$ ./oom-killer-try 300000 1
./oom-killer-try: PID 8498
0804a000-0806d000 rw-p 0804a000 00:00 0          [heap]
0...
5000...
0804a000-0a774000 rw-p 0804a000 00:00 0          [heap]
10000...
0804a000-0ce9c000 rw-p 0804a000 00:00 0          [heap]
15000...
0804a000-0f5a0000 rw-p 0804a000 00:00 0          [heap]
20000...
0804a000-11cc8000 rw-p 0804a000 00:00 0          [heap]
25000...
0804a000-143cd000 rw-p 0804a000 00:00 0          [heap]
30000...
0804a000-16af5000 rw-p 0804a000 00:00 0          [heap]
35000...
0804a000-1921d000 rw-p 0804a000 00:00 0          [heap]
40000...
0804a000-1b921000 rw-p 0804a000 00:00 0          [heap]
45000...
0804a000-1e04a000 rw-p 0804a000 00:00 0          [heap]
50000...
0804a000-20750000 rw-p 0804a000 00:00 0          [heap]
55000...
0804a000-22e76000 rw-p 0804a000 00:00 0          [heap]
60000...
0804a000-2559e000 rw-p 0804a000 00:00 0          [heap]
65000...
0804a000-27ca3000 rw-p 0804a000 00:00 0          [heap]
70000...
0804a000-2a3cb000 rw-p 0804a000 00:00 0          [heap]
75000...
0804a000-2cad1000 rw-p 0804a000 00:00 0          [heap]
80000...
0804a000-2f1f7000 rw-p 0804a000 00:00 0          [heap]
85000...
0804a000-3191f000 rw-p 0804a000 00:00 0          [heap]
90000...
0804a000-34026000 rw-p 0804a000 00:00 0          [heap]
95000...
0804a000-3674c000 rw-p 0804a000 00:00 0          [heap]
100000...
0804a000-38e52000 rw-p 0804a000 00:00 0          [heap]
105000...
0804a000-3b57a000 rw-p 0804a000 00:00 0          [heap]
110000...
0804a000-3dca1000 rw-p 0804a000 00:00 0          [heap]
115000...
0804a000-403a7000 rw-p 0804a000 00:00 0          [heap]
120000...
125000...
130000...
135000...
140000...
145000...
150000...
155000...
160000...
165000...
170000...
175000...
180000...
185000...
190000...
195000...
200000...
205000...
210000...
215000...
220000...
225000...
230000...
235000...
Killed
$ 
$ cat /var/log/messages
...
...
Nov  2 11:52:12 slesk kernel: oom-killer: gfp_mask=0x201d2, order=0
Nov  2 11:52:13 slesk kernel:  [<c01447f6>] out_of_memory+0x25/0x13f
Nov  2 11:52:13 slesk kernel:  [<c0145e0f>] __alloc_pages+0x1f3/0x2a5
Nov  2 11:52:13 slesk kernel:  [<c0147375>] __do_page_cache_readahead+0xc4/0x1e2
Nov  2 11:52:13 slesk kernel:  [<c0118d94>] __wake_up_common+0x2f/0x53
Nov  2 11:52:13 slesk kernel:  [<c011ad45>] __wake_up+0x2a/0x3d Nov  2 11:52:13 slesk kernel: oom-killer: gfp_mask=0x200d2, order=0
Nov  2 11:52:13 slesk kernel:  [<c01447f6>] out_of_memory+0x25/0x13f Nov  2 11:52:13 slesk kernel:  [<c0143f8c>] filemap_nopage+0x14f/0x2f7
Nov  2 11:52:13 slesk kernel:  [<c0145e0f>] __alloc_pages+0x1f3/0x2a5
Nov  2 11:52:13 slesk kernel:  [<c014c3d2>] __handle_mm_fault+0x2a6/0x7ea
Nov  2 11:52:13 slesk kernel:  [<c0152139>] read_swap_cache_async+0x2f/0xac Nov  2 11:52:13 slesk kernel:  [<c014b33b>] swapin_readahead+0x3a/0x58
Nov  2 11:52:14 slesk kernel:  [<c029131e>]  [<c014c667>] __handle_mm_fault+0x53b/0x7eado_page_fault+0x16e/0x525
Nov  2 11:52:14 slesk kernel:
Nov  2 11:52:14 slesk kernel:  [<c02911b0>] do_page_fault+0x0/0x525
Nov  2 11:52:15 slesk kernel:  [<c0104e5f>]  [<c029131e>] error_code+0x4f/0x60
Nov  2 11:52:15 slesk kernel: do_page_fault+0x16e/0x525
Nov  2 11:52:15 slesk kernel:  [<c02911b0>] do_page_fault+0x0/0x525
Nov  2 11:52:15 slesk kernel:  [<c0104e5f>] <6>Mem-info:
Nov  2 11:52:16 slesk kernel: error_code+0x4f/0x60DMA per-cpu:
Nov  2 11:52:16 slesk kernel:
Nov  2 11:52:16 slesk kernel: cpu 0 hot: high 0, batch 1 used:0
Nov  2 11:52:16 slesk kernel: cpu 0 cold: high 0, batch 1 used:0
Nov  2 11:52:16 slesk kernel: cpu 1 hot: high 0, batch 1 used:0
Nov  2 11:52:16 slesk kernel: cpu 1 cold: high 0, batch 1 used:0
Nov  2 11:52:16 slesk kernel: DMA32 per-cpu: empty
Nov  2 11:52:16 slesk kernel: Normal per-cpu:
Nov  2 11:52:17 slesk kernel: cpu 0 hot: high 186, batch 31 used:173
Nov  2 11:52:17 slesk kernel: cpu 0 cold: high 62, batch 15 used:24
Nov  2 11:52:17 slesk kernel: cpu 1 hot: high 186, batch 31 used:30
Nov  2 11:52:17 slesk kernel: cpu 1 cold: high 62, batch 15 used:38
Nov  2 11:52:17 slesk kernel: HighMem per-cpu: [<c01206bb>]
Nov  2 11:52:17 slesk kernel: cpu 0 hot: high 42, batch 7 used:41
Nov  2 11:52:17 slesk kernel: cpu 0 cold: high 14, batch 3 used:12
Nov  2 11:52:18 slesk kernel: do_syslog+0x154/0x341
Nov  2 11:52:18 slesk kernel: cpu 1 hot: high 42, batch 7 used:36
Nov  2 11:52:18 slesk kernel: cpu 1 cold: high 14, batch 3 used:2
Nov  2 11:52:18 slesk kernel: Free pages:       12020kB (120kB HighMem)
Nov  2 11:52:18 slesk kernel: Active:123649 inactive:124211 dirty:0 writeback:0 unstable:0 free:3005 slab:3589 mapped:247793 pagetables:877
Nov  2 11:52:18 slesk kernel: DMA free:4092kB min:68kB low:84kB high:100kB active:4904kB inactive:4220kB present:16384kB pages_scanned:15926 all_unreclaimable? yes
Nov  2 11:52:18 slesk kernel:  [<c0130977>] lowmem_reserve[]: 0autoremove_wake_function+0x0/0x2d
Nov  2 11:52:18 slesk kernel:  0 880 1006
Nov  2 11:52:18 slesk kernel: DMA32 free:0kB min:0kB low:0kB high:0kB active:0kB inactive:0kB present:0kB pages_scanned:0 all_unreclaimable? no
Nov  2 11:52:18 slesk kernel: lowmem_reserve[]: 0 0 880 1006

****************************************/
