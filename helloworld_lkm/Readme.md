TIP
On Ubuntu 22.04 or earlier, it's possible that the module build fails with this error:

`gcc-11: error: unrecognized command-line option ‘-ftrivial-auto-var-init=zero’`

It's a limitation within GCC ver 11; to fix it, install gcc-12, set CC to it in the Makefile and retry:

`sudo apt install gcc-12`

In the Makefile, add this line:

`CC := gcc-12`

`make`
