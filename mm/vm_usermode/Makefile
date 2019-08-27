# Makefile
ALL := malloc_brk_test segv_pgfault vm_user
CC := ${CROSS_COMPILE}gcc

all: ${ALL}
malloc_brk_test: malloc_brk_test.c
	${CC} malloc_brk_test.c -g -O0 -o malloc_brk_test -Wall
hello: hello.c
	${CC} hello.c -o hello -Wall
segv_pgfault: segv_pgfault.c
	${CC} segv_pgfault.c -o segv_pgfault -Wall
vm_user: vm_user.c
	${CC} vm_user.c -o vm_user -Wall

clean:
	rm -f *.o ${ALL}

