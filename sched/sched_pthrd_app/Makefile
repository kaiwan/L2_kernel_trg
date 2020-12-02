# Makefile for sched_pthread
sched_pthrd: sched_pthrd.c
	gcc sched_pthrd.c -o sched_pthrd -Wall -Wextra -O0 -lpthread
	sudo setcap CAP_SYS_NICE+eip ./sched_pthrd
	getcap ./sched_pthrd
clean:
	rm -fv sched_pthrd
