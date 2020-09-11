# Makefile
ALL := reg_fileio mmap_fileio

all: ${ALL}
CB_FILES := *.[ch]

reg_fileio: reg_fileio.c
	gcc reg_fileio.c -o reg_fileio -Wall
mmap_fileio: mmap_fileio.c
	gcc mmap_fileio.c -o mmap_fileio -Wall #-DDEBUG

# CB - C Beautifier ! uses indent- "beautifies" C code into the "Linux kernel style"
# Note- original source file(s) is overwritten
cb: ${CB_FILES}
	mkdir bkp 2> /dev/null; cp -f ${CB_FILES} bkp/
	indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1 ${CB_FILES}
clean:
	rm -f ${ALL}

