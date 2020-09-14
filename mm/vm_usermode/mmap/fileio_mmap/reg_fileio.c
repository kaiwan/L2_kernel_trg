/*
 * reg_fileio.c
 * 
 * History:
 *
 * Author(s) : 
 * License(s):
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "restart_lib.h"

/*---------------- Macros -------------------------------------------*/

/*---------------- Typedef's, constants, etc ------------------------*/

/*---------------- Functions ----------------------------------------*/

int main(int argc, char **argv)
{
	int fd_from, fd_to, n;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s source-file dest-file\n", argv[0]);
		exit(1);
	}

	fd_from = r_open2(argv[1], O_RDONLY);
	if (-1 == fd_from) {
		perror("open: from file");
		exit(1);
	}

	fd_to =
	    r_open3(argv[2], O_WRONLY | O_CREAT,
		    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (-1 == fd_to) {
		perror("open: to file");
		exit(1);
	}

	n = copyfile(fd_from, fd_to);
	printf("n = %d\n", n);

	close(fd_from);
	close(fd_to);
	exit(0);
}

/* vi: ts=4 */
