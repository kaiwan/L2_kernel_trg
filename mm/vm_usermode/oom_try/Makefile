# Makefile
ALL := oom-killer-try rdpg_oom_try
CC := ${CROSS_COMPILE}gcc

all: ${ALL}
oom-killer-try: oom-killer-try.c
	${CC} oom-killer-try.c -o oom-killer-try -Wall
rdpg_oom_try: rdpg_oom_try.c
	${CC} rdpg_oom_try.c -o rdpg_oom_try -Wall

debug:
oom-killer-try-dbg: oom-killer-try.c
	${CC} oom-killer-try.c -o oom-killer-try-dbg -Wall -O0 -DDEBUG -g -ggdb
clean:
	rm -f *.o ${ALL}
