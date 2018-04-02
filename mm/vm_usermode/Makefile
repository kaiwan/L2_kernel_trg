# Makefile
ALL := malloc_brk_test segv_pgfault cache_effect_try
CC := ${CROSS_COMPILE}gcc

all: ${ALL}
malloc_brk_test: malloc_brk_test.c
	${CC} malloc_brk_test.c -g -O0 -o malloc_brk_test -Wall
hello: hello.c
	${CC} hello.c -o hello -Wall
segv_pgfault: segv_pgfault.c
	${CC} segv_pgfault.c -o segv_pgfault -Wall

cache_effect_try: cache_effect_try.c
	${CC} cache_effect_try.c -o cache_effect_try -Wall -Os
dbg_cache_effect_try: cache_effect_try.c
	${CC} cache_effect_try.c -o dbg_cache_effect_try -Wall -O0 -g -ggdb

clean:
	rm -f *.o ${ALL}

