/*********************************************************************
 *
 * Copyright (C) 2020 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

/* ===============================
 * Sources Used:
 * https://classes.soe.ucsc.edu/cse130/Winter20/SECURE/print-ext-ascii.c
 * https://stackoverflow.com/questions/27023697/c-scandir-only-fill-namelist-with-folders
 * https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
 * https://www.gnu.org/software/libc/manual/html_node/Scanning-Directory-Content.html#Scanning-Directory-Content
 * https://linux.die.net/man/3/scandir
 * https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
 * http://codewiki.wikidot.com/system-calls
 * https://www.ibm.com/support/knowledgecenter/SSLTBW_2.2.0/com.ibm.zos.v2r2.bpxbd00/rtlse.htm
 * ================================*/

#include "fileman.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

// For file fd printing
#include <stdio.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
void fileman_dir_show(int fd, char *dname, int level_deep, bool fancy_char);
void fileman_write_line(int fd, char *entry_name, int levels_deep, bool fancy_char, bool is_first, bool is_last);


#define TAB "    "
#define SHORT_TAB "   "
#define SPACE " "
#define TEE "\u251C" // ├
#define HOR "\u2500" // ─
#define VER "\u2502" // |
#define ELB "\u2514" // └

#define V_TAB VER SHORT_TAB
#define T_TAB TEE HOR HOR SPACE
#define E_TAB ELB HOR HOR SPACE

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
	
	// Find data size of src file
	struct stat src_stats;
    int stats_r = stat(fsrc, &src_stats);
    
	// ERROR: Couldnt get stats
	if(stats_r == -1) return -1;

	// Extract size + make new read buffer
	int src_size = src_stats.st_size;
	char* buf = malloc(src_size);

	// Read -> Write
	fileman_read(fsrc, 0, buf, 0, src_size);
	fileman_write(fdest, 0, buf, 0, src_size);

	return src_size;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
void fileman_dir(int fd, char *dname)
{
	// Print the top level directory
	dprintf(fd, "%s\n", dname);
	fileman_dir_show(fd, dname, 0, false);
}

/*
 * You need to implement this function, see fileman.h for details 
 */
void fileman_tree(int fd, char *dname)
{
	// Print the top level directory
	dprintf(fd, "%s\n", dname);
	fileman_dir_show(fd, dname, 0, true);
}

void fileman_dir_show(int fd, char *dname, int levels_deep, bool fancy_char) {

	// List to store
	struct dirent **file_list;

	// Change into data.dir
	chdir(dname);

	// Scan files in the CURRENT dir & alpha sort
   	int num_items = scandir(".", &file_list, NULL, alphasort);
    
	// ERROR: something went wrong with items
	if(num_items == -1) return;

	// Loop through ach entry
	bool is_first = true;
	for(int i=0; i<num_items; i++) {
		struct dirent *entry = file_list[i];

		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

		// Write the current line
		fileman_write_line(fd, entry->d_name, levels_deep+1, fancy_char, is_first, (i == num_items - 1));
		is_first = false;

		// File is actually a directory
		if(entry->d_type == DT_DIR) {
			fileman_dir_show(fd, entry->d_name, levels_deep+1, fancy_char);
			chdir("..");
		}
	}

	// Done
	return;
}

void fileman_write_line(int fd, char *entry_name, int levels_deep, bool fancy_char, bool is_first, bool is_last) {

	// First print the required number of spacing
	for(int i=0; i<levels_deep; i++){
		if(fancy_char){
			// is the last thing to print
			if(i == levels_deep-1){
				if(is_last) dprintf(fd, "%s", E_TAB);
				else dprintf(fd, "%s", T_TAB);
			} else if(i==0){
				if(is_first) dprintf(fd, "%s", V_TAB);
				else if(is_last) dprintf(fd, "%s", TAB);
				else dprintf(fd, "%s", V_TAB);
			} else {
				///
			}
			
		}
		else{
			dprintf(fd, "%s", TAB);
		}
	}

	// Add the entry name
	dprintf(fd, "%s", entry_name);

	// Finish with a newl ine
	dprintf(fd, "\n");
	
}

