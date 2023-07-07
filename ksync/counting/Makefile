# Makefile
# A 'better' Makefile template for Linux Pthreads programming
#
# Besides the 'usual' targets to build production and debug versions of the
# code and cleanup, we incorporate targets to do useful (and indeed required)
# stuff like:
#  - prod_2part: build a '2-part' production target 'prod_2part'; it's
#     -O2, no debug symbolic info, strip-debug;
#     Excellent for production as it gives ability to debug as and when required!
#  - indent: adhering to (Linux kernel) coding style guidelines (indent+checkpatch)
#  - sa: static analysis target (via flawfinder, cppcheck)
#  - dynamic analysis target: via valgrind
#  -         + code coverage via gcov
#  - a packaging (.tar.xz) target and
#  - a help target.
#
# You will require these utils installed:
#  indent, flawfinder, valgrind, kernel-headers package -or- simply the
#  checkpatch.pl script, gcov, tar; + libasan
#
# To get started, just type:
#  make help
#
# (c) 2020 Kaiwan N Billimoria, kaiwanTECH
# License: MIT

## Pl check and keep or remove <foo>_dbg_[asan|ub|msan] targets
## (where <foo> is the program name) as desired.
ALL :=  prod prod_2part

###
# Update as required
# Simply replace the 'cr8ManyThrds' program name with your program name!
# f.e. with sed:
#   sed --in-place "s/cr8ManyThrds/myprg/g" Makefile
# Of course, if you have >1 C program to build, you must add  it manually,
# or better, keep one Makefile per program in separate directories.
###
ALL_NM :=  counting_5.1 counting_5.1_dbg counting_5.1_dbg_asan counting_5.1_dbg_ub

CC=${CROSS_COMPILE}gcc
CL=${CROSS_COMPILE}clang
STRIP=${CROSS_COMPILE}strip

PROD_OPTLEVEL=-O2
  # or -O3 or -Os
CFLAGS=-Wall -UDEBUG ${PROD_OPTLEVEL}
# Dynamic analysis includes the compiler itself!
# Especially the powerful Address Sanitizer (ASAN) toolset
CFLAGS_DBG=-g -ggdb -gdwarf-4 -O0 -Wall -Wextra -DDEBUG
CFLAGS_DBG_ASAN=${CFLAGS_DBG} -fsanitize=address
CFLAGS_DBG_UB=${CFLAGS_DBG} -fsanitize=undefined
#CFLAGS_DBG_MSAN=${CFLAGS_DBG} -fsanitize=memory

CFLAGS_GCOV=${CFLAGS_DBG} -fprofile-arcs -ftest-coverage

# Required vars
all: ${ALL}
SRC_FILES := *.[ch]
INDENT := indent
FLAWFINDER := flawfinder
CPPCHECK := cppcheck
VALGRIND := valgrind
# update as required
PKG_NAME := cr8ManyThrds
CHECKPATCH := ../../checkpatch.pl

# Targets and their rules
# Three types:
# 1. 'regular' production target 'prod': -O2, no debug symbolic info, stripped
# 2. '2-part' production target 'prod_2part': -O2, no debug symbolic info, strip-debug;
#     excellent for production as it gives ability to debug as and when required!
#     (internally invokes the 'debug' target as it requires the debug binary as well
# 3. 'debug' target(s): -O0, debug symbolic info (-g -ggdb), not stripped
prod: counting_5.1.c
	@echo
	@echo "--- building 'production'-ready target (-O2, no debug, stripped) ---"
	@echo
	${CC} ${CFLAGS} counting_5.1.c -o counting_5.1 -pthread
	${STRIP} --strip-all ./counting_5.1

# The '2-part executable' solution : use strip and objcopy to generate a
# binary executable that has the ability to retrieve debug symbolic information
# from the 'debug' binary!
prod_2part: counting_5.1.c
	@echo
	@echo "--- building 'production'-ready 2-part target (-O2, no debug, strip-debug) ---"
	@echo
# We reuire the 'debug' build for the 2part, so do that first
	make debug
	${CC} ${CFLAGS} counting_5.1.c -o counting_5.1 -pthread
	${STRIP} --strip-debug ./counting_5.1
	objcopy --add-gnu-debuglink=./counting_5.1_dbg ./counting_5.1

debug: counting_5.1.c
	@echo
	@echo "--- building 'debug'-ready targets (with debug symbolic info, not stripped) ---"
	@echo
	${CC} ${CFLAGS_DBG} counting_5.1.c -o counting_5.1_dbg -pthread
#-- Sanitizers (use clang or gcc)
	${CC} ${CFLAGS_DBG_ASAN} counting_5.1.c -o counting_5.1_dbg_asan -pthread
	${CC} ${CFLAGS_DBG_UB} counting_5.1.c -o counting_5.1_dbg_ub -pthread
#	${CC} ${CFLAGS_DBG_MSAN} cr8ManyThrds.c -o cr8ManyThrds_dbg_msan -pthread


#--------------- More (useful) targets! -------------------------------

# indent- "beautifies" C code - to conform to the the Linux kernel
# coding style guidelines.
# Note! original source file(s) is overwritten, so we back it up.
# code-style : "wrapper" target over the following kernel code style targets
code-style:
	make indent
	make checkpatch

indent: ${SRC_FILES}
	make clean
	@echo
	@echo "--- applying Linux kernel code-style indentation with indent ---"
	@echo
	mkdir bkp 2> /dev/null; cp -f ${SRC_FILES} bkp/
	${INDENT} -linux ${SRC_FILES}

checkpatch:
	make clean
	@echo
	@echo "--- applying Linux kernel code-style checking with checkpatch.pl ---"
	@echo
	${CHECKPATCH} -f --no-tree --max-line-length=95 ${SRC_FILES}

# sa : "wrapper" target over the following static analyzer targets
sa:   # static analysis
	make sa_flawfinder
	make sa_cppcheck

# static analysis with flawfinder
sa_flawfinder:
	make clean
	@echo
	@echo "--- static analysis with flawfinder ---"
	@echo
	${FLAWFINDER} --neverignore --context *.[ch]

# static analysis with cppcheck
sa_cppcheck:
	make clean
	@echo
	@echo "--- static analysis with cppcheck ---"
	@echo
	${CPPCHECK} -v --force --enable=all -i bkp/ --suppress=missingIncludeSystem .

# dynamic analysis with valgrind
valgrind:
	make debug
	@echo
	@echo "--- dynamic analysis with valgrind ---"
	@echo
	${VALGRIND} --tool=memcheck --trace-children=yes ./cr8ManyThrds_dbg

# Testing: line coverage with gcov(1)
covg:
	@echo
	@echo "=== Code coverage with gcov ==="
	@echo
	${CC} ${CFLAGS_GCOV} cr8ManyThrds.c -o cr8ManyThrds_gcov -pthread
# update as required (param)
	./cr8ManyThrds_gcov 5
	gcov ${SRC_FILES}
	@echo "Examine the *.c.gcov file(s)"

# packaging
package:
	@echo
	@echo "--- packaging ---"
	@echo
	rm -f ../${PKG_NAME}.tar.xz
	make clean
	tar caf ../${PKG_NAME}.tar.xz *
	ls -l ../${PKG_NAME}.tar.xz
	@echo "=== $(PKG_NAME).tar.xz package created ==="

clean:
	@echo
	@echo "--- cleaning ---"
	@echo
	rm -vf ${ALL_NM} core* vgcore* *.o *~ *.c.gcov *.gcda *.gcno

help:
	@echo '=== Makefile Help : additional targets available ==='
	@echo
	@echo 'TIP: type make <tab><tab> to show all valid targets'
	@echo

	@echo 'Regular targets ::'
	@echo ' 1. 'prod'  : regular production target: -O2, no debug symbolic info, stripped'
	@echo ' 2. 'debug' : -O0, debug symbolic info (-g -ggdb), not stripped'
	@echo ' 3. 'prod_2part': production target : -O2, no debug symbolic info, strip-debug; \
    Excellent for production as it gives ability to debug as and when required! \
    (shown as third option as it *requires* the 'debug' build'
	@echo
	@echo 'Doing a 'make' will build all three shown above.'

	@echo
	@echo '--- code style targets ---'
	@echo 'code-style : "wrapper" target over the following kernel code style targets'
	@echo ' indent     : run the $(INDENT) utility on source file(s) to indent them as per the kernel code style'
	@echo ' checkpatch : run the kernel code style checker tool on source file(s)'

	@echo
	@echo '--- static analyzer targets ---'
	@echo 'sa          : "wrapper" target over the following static analyzer targets'
	@echo ' sa_flawfinder : run the static analysis flawfinder tool on the source file(s)'
	@echo ' sa_cppcheck   : run the static analysis cppcheck tool on the source file(s)'

	@echo
	@echo '--- dynamic analysis targets ---'
	@echo ' valgrind   : run the dynamic analysis tool ($(VALGRIND)) on the binary executable'
	@echo ' covg       : run the gcov code coverage tool on the source'

	@echo
	@echo '--- misc targets ---'
	@echo ' clean      : cleanup - remove all the binaries, core files, etc'
	@echo ' package    : tar and compress the source files into the dir above'
	@echo ' help       : this 'help' target'
