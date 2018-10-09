#ifndef FUTIL_H
#define FUTIL_H

#include <stdio.h>

#define SIGMA   	128     // size of code set
#define SIGMA_BEG	1		// first address of characters preseverd for delimiter
#define SIGMA_END	127		// last address of characters

/*******************************************
* Supposing file_size f and block_size b,
* Let index file smaller than original file
* (f / b) * 128 * 4 <= f
* b >= 512 
********************************************/
#define MIN_FSIZE_TO_BE_STORED	512
#define BLOCK_SIZE 				512

extern char delim;
extern int  is_write;

int  get_f_size(FILE *fp);
void generate_table(int *C, int **Occ, FILE *enf, FILE *idf);
int  extract_occ(int c, int pos, FILE *enf, FILE *idf);

#endif //FUTIL_H
