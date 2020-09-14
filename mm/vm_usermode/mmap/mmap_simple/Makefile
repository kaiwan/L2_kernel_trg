# Makefile
ALL := mmap_simple

all: ${ALL}
CB_FILES := *.[ch]

mmap_simple: mmap_simple.c ../../../../convenient.h
	gcc mmap_simple.c -o mmap_simple -Os -Wall #-DDEBUG

# CB - C Beautifier ! uses indent- "beautifies" C code into the "Linux kernel style"
# Note- original source file(s) is overwritten
cb: ${CB_FILES}
	mkdir bkp 2> /dev/null; cp -f ${CB_FILES} bkp/
	indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1 ${CB_FILES}
clean:
	rm -f ${ALL}

