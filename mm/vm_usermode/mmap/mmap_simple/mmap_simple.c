/*
 * mmap_simple.c
 * 
 * mmap() a file and display the contents as asked.
 * Private file-mapped I/O.
 *
 * History:
 *
 * Author(s) : Kaiwan Billimoria <kaiwan -at- kaiwantech -dot- com>
 * License(s): [L]GPL v2
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "restart_lib.h"
#include "../../../../convenient.h"

/*--------------- Function hex_dump() sourced from:
http://www.alexonlinux.com/hex-dump-functions
All rights rest with original author(s).----------------------

Added an start-offset and a 'verbose' parameter..(kaiwan).
*/
void hex_dump(unsigned char *data, int offset, int size, char *caption, int verbose)
{
	int i;			// index in data...
	int j;			// index in line...
	char temp[8];
	char buffer[128];
	char *ascii;

	memset(buffer, 0, 128);

	if (verbose && caption)
		printf("---------> %s <--------- (%d bytes from %p)\n", caption,
		       size, data);

	// Printing the ruler...
	printf
	    ("        +0          +4          +8          +c            0   4   8   c   \n");

	// Hex portion of the line is 8 (the padding) + 3 * 16 = 52 chars long
	// We add another four bytes padding and place the ASCII version...
	ascii = buffer + 58;
	memset(buffer, ' ', 58 + 16);

	sprintf(temp, "+%06d", offset);
	buffer[58 + 16] = '\n';
	buffer[58 + 17] = '\0';
	buffer[0] = '+';

	// Set offset to initial offset
	buffer[1] = temp[1];
	buffer[2] = temp[2];
	buffer[3] = temp[3];
	buffer[4] = temp[4];
	buffer[5] = temp[5];
	buffer[6] = temp[6];

	for (i = 0, j = 0; i < size; i++, j++) {
		if (j == 16) {
			printf("%s", buffer);
			memset(buffer, ' ', 58 + 16);

			sprintf(temp, "+%06d", i+offset); // set offset to initial offset
			memcpy(buffer, temp, 7);

			j = 0;
		}

		sprintf(temp, "%02x", 0xff & data[i]);
		memcpy(buffer + 8 + (j * 3), temp, 2);
		if ((data[i] > 31) && (data[i] < 127))
			ascii[j] = data[i];
		else
			ascii[j] = '.';
	}

	if (j != 0)
		printf("%s", buffer);
}


int main(int argc, char **argv)
{
	int fd_from;
	off_t fsz, len, off = 0;
	struct stat sstat;
	void *data_src, *origptr;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s source-file start_offset length\n",
			argv[0]);
		exit(1);
	}

	fd_from = r_open2(argv[1], O_RDONLY);
	if (-1 == fd_from) {
		perror("open: src file");
		exit(1);
	}
	// Query src file size
	if (fstat(fd_from, &sstat) == -1) {
		perror("fstat");
		exit(1);
	}
	fsz = sstat.st_size;
	if (0 == fsz) {
		fprintf(stderr,
			"%s: source-file %s size is zero bytes, aborting...\n",
			argv[0], argv[1]);
		exit(1);
	}

	len = atol(argv[3]);
	if (len <= 0) {
		fprintf(stderr,
			"%s: invalid length %ld, aborting...\n", argv[0], len);
		exit(1);
	}
	off = atol(argv[2]);
	if ((off < 0) || ((off+len) > fsz)) {
		fprintf(stderr,
			"%s: invalid offset or offset/length combination, aborting...\n", argv[0]);
		exit(1);
	}

	/*
	   void *mmap(void *addr, size_t length, int prot, int flags,
	   int fd, off_t offset);

		A private mapping (obtained by using the MAP_PRIVATE flag), will initialize 
		the mapped memory to the file region being mapped; any modifications to the 
		mapped region are _not_ carried through to the file, though.
	 */
	origptr = data_src = mmap(0, fsz, PROT_READ, MAP_PRIVATE, fd_from, 0);	//off);
	if (data_src == MAP_FAILED) { // (void *)-1) {
		perror("mmap: to src file");
		exit(1);
	}

	// void hex_dump(unsigned char *data, int offset, int size, char *caption, int verbose)
	hex_dump(data_src + off, off, len, "Data", 0);

	/*
	int msync(void *addr, size_t length, int flags);

	   msync() flushes changes made to the in-core copy of a file that was 
       mapped into memory using mmap(2) back to disk.  Without use of this call there
       is no guarantee that changes are written back before munmap(2) is called.  
       To be more precise, the part of the file that corresponds to the  memory
       area starting at addr and having length length is updated.

       The  flags  argument  may  have  the  bits MS_ASYNC, MS_SYNC, and MS_INVALIDATE set, 
       but not both MS_ASYNC and MS_SYNC.  MS_ASYNC specifies that an update be scheduled, 
       but the call returns immediately.  MS_SYNC asks for an update and waits for it to complete.  
       MS_INVALIDATE asks to invalidate other mappings of the same file (so that 
       they can be updated with the fresh values just written).
	*/
	if (msync (origptr, len, MS_SYNC) < 0) // redundant here as we're calling munmap() immd ..
		perror("mysnc");

	if (munmap(origptr, len) == -1) {
		perror("munmap");
		exit(1);
	}
	close(fd_from);
	exit(0);
}

/* vi: ts=4 */
