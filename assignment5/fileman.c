/*********************************************************************
 *
 * Copyright (C) 2020 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

#include "fileman.h"

#include <unistd.h>
#include <fcntl.h>

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_read(char *fname, size_t foffset, char *buf, size_t boffset, size_t size) {
	
	// Obtain a file desc
	int file_desc = open(fname, O_RDONLY);

	// ERROR: File does not exist
	if(file_desc == -1) return -1;

	// Attempt to adjust the file offset
	off_t file_offset = lseek(file_desc, foffset, SEEK_SET);

	// ERROR: lseek
	if(file_offset == -1) return -1;

	// We can now attempt to read...
	ssize_t read_size = read(file_desc, buf + boffset, size);

	// ERROR: read
	if(read_size < 0) return -1;

	// Was able to read n > 0 bytes
	return read_size;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_write(char *fname, size_t foffset, char *buf, size_t boffset, size_t size) {
	return 0;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_append(char *fname, char *buf, size_t size) {
	return 0;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_copy(char *fsrc, char *fdest) {
	return 0;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
void fileman_dir(int fd, char *dname)
{
}

