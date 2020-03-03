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
#include <sys/types.h>
#include <sys/stat.h>

// For error stuff
// #include <errno.h>
// #include <string.h>
// #include <stdio.h>

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
	if(file_offset == -1) {
		close(file_desc);
		return -1;
	}

	// We can now attempt to read...
	ssize_t read_size = read(file_desc, buf + boffset, size);

	// ERROR: read
	if(read_size < 0) {
		close(file_desc);
		return -1;
	}

	// Close the file
	close(file_desc);

	// Was able to read n > 0 bytes
	return read_size;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_write(char *fname, size_t foffset, char *buf, size_t boffset, size_t size) {

	// Obtain a file desc
	int file_desc = open(fname, O_WRONLY);
 
	// ERROR: File exisits already
    if (file_desc != -1) {
		close(file_desc); // Make sure to close the file
		return -1;
	}

	// File doesnt exist so create a new file desc
	file_desc = open(fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);

	// ERROR: File could not be created
    if (file_desc == -1) {
		close(file_desc); // Make sure to close the file
		return -1;
	}

	// Attempt to adjust the file offset
	off_t file_offset = lseek(file_desc, foffset, SEEK_SET);

	// ERROR: lseek
	if(file_offset == -1) {
		close(file_desc);
		return -1;
	}

	// Attempt to write
	ssize_t wrote_size = write(file_desc, buf + boffset, size);

	// ERROR: Something went wrong OR wrote wrong amount of data
	if(wrote_size < 0 || size != wrote_size) {
		close(file_desc); // Close file
		return -1;
	}

	// Close file
	close(file_desc);

	// Wrote n bytes
    return wrote_size;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_append(char *fname, char *buf, size_t size) {
	
	// Obtain a file desc
	int file_desc = open(fname, O_APPEND | O_WRONLY);
 
	// ERROR: File does not exist
    if (file_desc == -1) {
		close(file_desc);
		return -1;
	}

	// Attempt to append
	ssize_t append_size = write(file_desc, buf, size);

	// ERROR: Something went wrong OR wrote wrong amount of data
	if(append_size < 0 || size != append_size) {
		close(file_desc); // Close file
		return -1;
	}

	// Close file
	close(file_desc);

	// Appended n bytes
    return append_size;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_copy(char *fsrc, char *fdest) {
	
	// Obtain a 2 file desc
	int src_file_desc = open(fsrc, O_RDONLY);
	int dest_file_desc = open(fdest, O_WRONLY);
 
	// ERROR: File couldnt be opedned or already exists
    if (src_file_desc == -1 || dest_file_desc != -1) {
		close(src_file_desc); // Make sure to close the file
		close(dest_file_desc); // Make sure to close the file
		return -1;
	}

	return 0;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
void fileman_dir(int fd, char *dname)
{
}

