# Makefile
ALL := hello
all: $(ALL)

hello: hello.s
	nasm -f elf64 -o hello.o hello.s
	ld -o hello hello.o
clean:
	rm -f hello hello.o
