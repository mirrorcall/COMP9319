#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "futil.h"


#define TRUE			1
#define FALSE			0
#define INF 			-1
#define MAX_REC_SIZE	5000


int  id_of_delim(int *delim_rank, int value, int num_delim);
int  retrieve_nth_delim(FILE *auf, int nth, int num_delim);
void strrev(char *x, int begin, int end);
int  next_occ(int **Occ, int curr, int pos);
int  next_char(int *C, int curr);
int  rmdup(int *x, int end);
int  cmper(const void *x, const void *y);


char delim;
int  is_write;

int 
main(int argc, char const *argv[])
{
    delim         = argv[1][0];
    char *en_path = strdup(argv[2]);
	// argv[3] assigned later since its relative folder path															
    char *option  = strdup(argv[4]);   							
    char *q_str   = strdup(argv[5]);

    /* dealing with newline (\n) as delimiter */
	if (strcmp(argv[1], "\\n") == 0)
		delim = '\n';

	/* modify correct path of storing temporary files */
	size_t au_nitem = strlen(argv[2]) + strlen(".aux") + 1;
	char   *au_path = malloc(au_nitem * sizeof(char));
	memset(au_path, 0, au_nitem);
	strcpy(au_path, argv[2]);
	strcat(au_path, ".aux\0");

	size_t id_nitem = strlen(argv[3]) + strlen("/occ.txt") + 1;
	char   *id_path = malloc(id_nitem * sizeof(char));
	memset(id_path, 0, id_nitem);
	strcpy(id_path, argv[3]);
	strcat(id_path, "/occ.txt\0");

    int i, j;
    FILE *auf  = fopen(au_path, "rb");
    FILE *enf  = fopen(en_path, "r");
    FILE *idfw = fopen(id_path, "w");
    int f_size = get_f_size(enf);
    
    int *C = calloc(SIGMA, sizeof(int));
    int **Occ = malloc(sizeof(int *) * SIGMA);
	for (i = 0; i < SIGMA; i++)
    	Occ[i] = calloc(sizeof(int) * BLOCK_SIZE, sizeof(int));
    if (f_size > MIN_FSIZE_TO_BE_STORED)
    	is_write = 1;
    else
    {
    	for (i = 0; i < SIGMA; i++)
        	Occ[i] = calloc(sizeof(int) * f_size, sizeof(int));
    	is_write = 0;
    }

    generate_table(C, Occ, enf, idfw);						  // idf need to be closed
    fclose(idfw);

/* debug info */
#ifdef DEBUG
printf("C TABLE:\n");
for (int l = 0; l < SIGMA; l++)
	if (C[l] != 0)
		printf("[%c]: %d\n", l, C[l]);
printf("\nOCCURANCE TABLE:\n");
for (int l = 0; l < SIGMA; l++)
	for (int g = 0; g < f_size; g++)
		if (Occ[l][g] != 0)
		{
			if (l == SIGMA_BEG)
				printf("%c,%d=%d\n", delim, g, Occ[l][g]);
			else
				printf("%c,%d=%d\n", l, g, Occ[l][g]);
		}
#endif
	
	FILE *idfr = fopen(id_path, "r");

	/**************************************
	* backward search starts here
	***************************************/
	unsigned char *P = (unsigned char*)q_str;
	int p;
	unsigned char c;
	int First, Last;
	int num_delim = get_f_size(auf) / sizeof(int);

	if (strcmp(option, "-i") == 0)							  // gives result of m-to-n records
	{
		char *record = malloc(MAX_REC_SIZE);
		int start, end;
		char *dup = strdup((char *)P);
		start = strtol(strtok(dup, " "), (char **)NULL, 10);
		end = strtol(strtok(NULL, " "), (char **)NULL, 10);
		int total_record = end - start + 1;
		i = 0;
		j = 0;
		int offset = start;
		int rec_no = start;
		
		if (end == num_delim)
			end = 1;
		while (i < total_record)
		{
			fseek(enf, offset-1, SEEK_SET);
			c = fgetc(enf);
			if (is_write)
			{
				//offset = C[c] + next_occ(Occ, c, offset-1) + 1;
				offset = C[c] + extract_occ(c, offset-1, enf, idfr) + 1;
				//printf("%d: %d\n", offset-1, next_occ(Occ, c, offset-1)==extract_occ(c, offset-1, enf, idfr));
			}
			else
				offset = C[c] + next_occ(Occ, c, offset-1) + 1;

				// printf("'%c' in %d\n", c, offset-1);
				// printf("%d\n", next_occ(Occ, c, offset-1)==extract_occ(c, offset-1, enf, idfr));				
				// printf("%d ", next_occ(Occ, c, offset-1));
				// printf("%d\n", extract_occ(c, offset-1, enf, idfr));

			if (c == delim)
			{
				record[j] = '\0';
				if (record[0] == 0)
					printf("\n");
				else
				{
					size_t rec_len = strlen(record) - 1;
					strrev(record, 0, rec_len);
					printf("%s\n", record);
					memset(record, 0, rec_len);					
				}
				rec_no++;
				offset = rec_no;
				i++;
				j = 0;
			}
			else
			{
				record[j] = c;
				j++;
			}
		}

		free(dup);
		free(record);
	}
	else
	{
		p = (int) strlen((char *)P);
		i = p;
		c = P[p - 1];
		First = C[c] + 1;

		if (C[c] != 0)
		{
		    int *delim_rank = malloc(num_delim * sizeof(int));
	    	fread(delim_rank, num_delim, sizeof(int), auf);

			if (next_char(C, c) == c)
				Last = First;
			else
				Last = C[next_char(C, c)];

		    while ((First <= Last) && (i >= 2))
		    {
		        c = P[i - 2];

		        if (is_write)
		        {
		        	First = C[c] + extract_occ(c, First-1, enf, idfr) + 1;
		        	Last = C[c] + extract_occ(c, Last, enf, idfr);
		        }
		        else
		        {
		        	First = C[c] + next_occ(Occ, c, First-1) + 1;
		        	Last = C[c] + next_occ(Occ, c, Last);
		        }
		        
		        i--;
		    }

		    if (strcmp(option, "-m") == 0)		// gives the number of all matching records
		    {

		    	if (Last < First)
		        	;//printf("0\n");
		    	else
		        	printf("%d\n", Last-First+1);
		    }
		    else									
		    {
			    if (Last >= First)
				{
			    	int *records = calloc(Last-First+1, sizeof(int));
			    	int j, k = 0;
				    int offset;
				    int record;
				    int result;

					// keep searching until reaching delimiter
				    for (j = First; j <= Last; j++)
				    {
				    	offset = j;
				    	while (TRUE)
				    	{
				    		fseek(enf, offset-1, SEEK_SET);
				    		c = fgetc(enf);

				    		if (c == delim)
				    		{
				    			/*
				    			 * Since backward search searches to the previous delimter
				    			 * E.g. obtaining 2 is actually representing third record
				    			 * Specially, last delimiter represents first record
				    			 */
				    			if ((record=id_of_delim(delim_rank, offset, num_delim)) != num_delim)
				    				records[k] = record + 1;
				    			else
				    				records[k] = 1;
				    			k++;
				 				break;
				    		}
				    	
				    		if (is_write)
				    			offset = C[c] + extract_occ(c, offset-1, enf, idfr) + 1;
			    			else
					    		offset = C[c] + next_occ(Occ, c, offset-1) + 1;
				    	}
				    }

				    result = rmdup(records, k);
					if (strcmp(option, "-n") == 0)		// gives the number of unique matching records
						printf("%d\n", result);
					else								// gives the result of appearing records
					{
						qsort(records, k, sizeof(int), cmper);
						for (j = 0; j < k; j++)
							if (records[j] != INF)
								printf("%d\n", records[j]);
					}
					free(records);
				}
		    }
		}

	}

    /* closing opening files */
	fclose(idfr);
    fclose(enf);
    fclose(auf);
    //flush(C, Occ, f_size);

    /* freeing buffers */
    free(au_path);
    free(q_str);
    free(option);
    free(en_path);

    free(C);
    if (!is_write)
	    for (i = 0; i < SIGMA; i++)
    	    free(Occ[i]);
    free(Occ);

    return EXIT_SUCCESS;
}

// next occ from memory
int 
next_occ(int **Occ, int curr, int pos)
{
	int ret_occ = 0;
	int i;
	for (i = pos; i >= 0; i--)
		if ((ret_occ=Occ[curr][i]) != 0)
			break;

	return ret_occ;
}

int 
next_char(int *C, int curr)
{
    int i;
    for (i = curr+1; i < SIGMA; i++)
        if (C[i] != 0)
            break;
    if (i == SIGMA)		// means curr is the final char
    	i = curr;

    return i;
}

void 
strrev(char *x, int begin, int end)
{
	char c;

   	if (begin >= end)
    	return;
 
	c          = *(x+begin);
   	*(x+begin) = *(x+end);
   	*(x+end)   = c;
 
   	strrev(x, ++begin, --end);
}

int 
rmdup(int *x, int end)
{
	int result = end;
	int i, j;

	for (i = 0; i < end; i++)
	{
		for (j = i+1; j < end; j++)
		{
			if (x[i] == x[j] && x[i] != INF)
			{
				x[j] = INF;
				result--;
			}
		}
	}

	return result;
}

int 
cmper(const void *x, const void *y)
{
	return (*(int *)x - *(int *)y);
}

int
id_of_delim(int *delim_rank, int value, int num_delim)
{
	int i;
	for (i = 0; i < num_delim; i++)
		if (delim_rank[i] == (value-1))
			break;

	return i + 1;
}

// start from 1
int 
retrieve_nth_delim(FILE *auf, int nth, int num_delim)
{
	int delim_rank_offset;
	fseek(auf, sizeof(int)*(nth-1), SEEK_SET);
	fread(&delim_rank_offset, sizeof(int), 1, auf);

	return delim_rank_offset;
}
