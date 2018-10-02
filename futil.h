#ifndef FUTIL_H
#define FUTIL_H

#include <stdio.h>

#define SIGMA   	128     // size of code set
#define SIGMA_BEG	1		// first address of characters preseverd for delimiter
#define SIGMA_END	127		// last address of characters

extern char delim;

int  get_f_size(FILE *fp);
void generate_table(int *C, int **Occ, FILE *enf);
void flush(int *C, int **Occ, int f_size);

#endif //FUTIL_H
