#include <stdio.h>
#include <stdlib.h>

#include "futil.h"

char delim;

int get_f_size(FILE *fp)
{
    int f_size = 0;

    fseek(fp, 0, SEEK_END);
    f_size = (int) ftell(fp);
    rewind(fp);

    return f_size;
}

/*
 * pass file_path into function
 * write c_table and occ_table out
 * need to be freed in main - call flush
 */
void generate_table(int *C, int **Occ, FILE *enf)
{
    int i, j;
    int c, s = 0;

    int *freq = calloc(SIGMA, sizeof(int));

    while ((c=fgetc(enf)) != EOF)
    {
        if (c == delim)
        {
            freq[SIGMA_BEG]++;
            if (Occ[SIGMA_BEG][s] == freq[SIGMA_BEG])
                continue;
            s++;
            Occ[SIGMA_BEG][s] = freq[SIGMA_BEG];
        }
        
        else
        {
            freq[c]++;
            if (Occ[c][s] == freq[c])
                continue;
            s++;
            Occ[c][s] = freq[c];
        }   
    }

    for (i = 0; i < SIGMA; i++)
        for (j = 0; j < i; j++)
            if (freq[i] != 0)
                C[i] += freq[j];

    // preserve last address for EOF
    C[SIGMA_END] = get_f_size(enf);

    rewind(enf);
}

void flush(int *C, int **Occ, int f_size)
{
    int i;
    for (i = 0; i < SIGMA; i++)
        free(Occ[i]);
    free(Occ);

    free(C);
}