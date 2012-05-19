/* 
 * Programmer : Nathan A. Mourey II
 * Program    : SimpleCrypt
 * Date       : October 21st 2011
 * Program    : Simple Crypt -- Inspired by CompTIA Security+ book.
 * Copyright  : GLPv3
 * Referance  : http://www.linuxquestions.org/questions/programming-9/c-howto-read-binary-file-into-buffer-172985/
 * 	      : for fseek and ftell functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* for preprocessor. */
#define MIN_PASS_LENGTH 10
#define MAX_PASS_LENGTH 128

#define INPUT_BUFFER 132

#define COPY "Simple Crypt v2.0 : Copyright (C) 2011, 2012 Nathan A. Mourey II <nmoureyii@ne.rr.com>"

/* global variable */
char pass_phrase[INPUT_BUFFER];

/* data stucture for CryptFile infomation. */
typedef struct CryptFile {
        char *data_in_buffer;
        char *data_out_buffer;
	char *pass;
	int pass_len;
	int chunk_size;
	int remaining;
	/* (data in size) length of data_in_buffer */
        int file_length;	
	struct stat stat_buff;
} CryptFile;

/* get users pass phrase. */
int get_pass(void)
{
	printf("Enter encryption key [between %i and %i charaters] : ", MIN_PASS_LENGTH, MAX_PASS_LENGTH);
	fgets(pass_phrase, sizeof(pass_phrase), stdin);

	if ( ((strlen(pass_phrase)-1) <= MIN_PASS_LENGTH) || ((strlen(pass_phrase)-1) >= MAX_PASS_LENGTH) ){
		fprintf(stderr, "Error : Encryption key must be longer that %i charaters and %i charaters or less.\n", 
		MIN_PASS_LENGTH, MAX_PASS_LENGTH);
		fprintf(stderr, "Error : No file written.\n");
		printf("%s\n", COPY);
		return -1;
	}
}

/* read a file into a buffer. */
void read_file(CryptFile *cf, char *file_in, char *file_out)
{
	int in_file, out_file;

	/* open input file. */
	if ( (in_file = open(file_in, O_RDONLY)) < 0) {
		fprintf(stderr, "Error could not open file : %s\n", file_in);
		exit(1);
	}

	/* fill stat_buff with file info. */
	fstat(in_file, &cf->stat_buff);

	/* open output file. */
	if  ( (out_file = open(file_out, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644)) < 0 ){
		fprintf(stderr, "Error could not open file : %s\n", file_out);
		exit(1);
	}

	/* verify that the output file is writeable. */
	lseek(out_file, cf->stat_buff.st_size-1, SEEK_SET);
	if (write(out_file, "", 1) != 1)
		fprintf(stderr, "Unable to write\n");
	
	cf->file_length = cf->stat_buff.st_size;
	
	/* mem map files.  */
	cf->data_in_buffer = mmap(0, cf->stat_buff.st_size, PROT_READ, MAP_SHARED, in_file, 0);
	cf->data_out_buffer = mmap(0, cf->stat_buff.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, out_file, 0);

	/* create working buffer */
	memcpy(cf->data_out_buffer, cf->data_in_buffer, cf->stat_buff.st_size);

	/* unmamp input buffer */
	munmap(cf->data_in_buffer, cf->stat_buff.st_size);
	
	/* close input file */
	close(in_file);
	close(out_file);
}

/* XOR encryption algo. */
void encrypt_data(CryptFile *cf)
{
	int i, j, k = 0;

	/* process each chunk. */
	for (i = 0; i < cf->chunk_size; i++){
		for(j = 0; j < cf->pass_len; j++){	
			cf->data_out_buffer[k] = cf->data_out_buffer[k] ^ cf->pass[j];
			k++;
		}
	}

	/* process the remaining data. */
	for (i = 0; i < cf->remaining; i++){
		cf->data_out_buffer[k] = cf->data_out_buffer[k] ^ cf->pass[i];
		k++;
	}
}
