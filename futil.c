#include <stdio.h>
#include <stdlib.h>

#include "futil.h"

char delim;
int  is_write;

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
 * need to be freed in main - <del>call flush</del>
 */
void generate_table(int *C, int **Occ, FILE *enf, FILE *idf)
{
    int i, j;
    int c, s = 0;
    int f_size = get_f_size(enf);
    int *freq = calloc(SIGMA, sizeof(int));
    int count = 0;
    while ((c=fgetc(enf)) != EOF)
    {
        if (c == delim)
        {
            freq[SIGMA_BEG]++;
            if (count < BLOCK_SIZE)
            {
                if (Occ[SIGMA_BEG][s] == freq[SIGMA_BEG])
                    continue;
                s++;
                Occ[SIGMA_BEG][s] = freq[SIGMA_BEG];
            }
        }
        else
        {
            freq[c]++;
            if (count < BLOCK_SIZE)
            {
                if (Occ[c][s] == freq[c])
                    continue;
                s++;
                Occ[c][s] = freq[c];
            }
        }
        
        count++;
        if (count % BLOCK_SIZE == 0)
            fwrite(freq, sizeof(int), SIGMA, idf);
    }

    for (i = 0; i < SIGMA; i++)
        for (j = 0; j < i; j++)
            if (freq[i] != 0)
                C[i] += freq[j];

    // preserve last address for EOF
    C[SIGMA_END] = f_size;
}

int extract_occ(int c, int pos, FILE *enf, FILE *idf)
{
    if (pos == 0)
        return 0;
    
    int occ = 0;
    int block_unit = pos / BLOCK_SIZE;
    int i = block_unit * BLOCK_SIZE;

    fseek(idf, BLOCK_SIZE*(block_unit-1)+c*4, SEEK_SET);
    fread(&occ, sizeof(int), 1, idf);
    // printf(" before:%d ", occ);
    fseek(enf, i, SEEK_SET);
    while(i < pos)
    {
        if (fgetc(enf) == c)
            occ++;
        i++;
    }
    rewind(idf);
    rewind(enf);

    return occ;
}
