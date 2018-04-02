
$ ./tst_fileio.sh 30
7680+0 records in
7680+0 records out
31457280 bytes (31 MB) copied, 2.88546 s, 10.9 MB/s
Regular file io test:

real	0m0.052s
user	0m0.000s
sys	0m0.052s
./tst_fileio.sh: line 24: unalias: ls: not found
-rw-rw-r-- 1 kaiwan kaiwan 31457280 Oct  7 17:03 /home/kaiwan/destfile
-rw-rw-r-- 1 kaiwan kaiwan 31457280 Oct  7 17:03 /home/kaiwan/srcfile
7680+0 records in
7680+0 records out
31457280 bytes (31 MB) copied, 2.88693 s, 10.9 MB/s
mmap-ed file io test:

real	0m0.049s
user	0m0.004s
sys	0m0.044s
./tst_fileio.sh: line 42: unalias: ls: not found
-rw-r--r-- 1 kaiwan kaiwan 31457280 Oct  7 17:03 /home/kaiwan/destfile
-rw-rw-r-- 1 kaiwan kaiwan 31457280 Oct  7 17:03 /home/kaiwan/srcfile
$ 

