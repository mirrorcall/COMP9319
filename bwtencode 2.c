#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "futil.h"

#define BUFFER_SIZE		1024*1024*50							// maximum input buffer size
#define INFINITY		1.0 / 0.0

static char *buffer;
static char delimiter;

struct delim_stat
{
	int delim_rank;												// sorted based on delim_rank_enty
	int delim_rank_entry;
};

typedef struct delim_stat ds;

int  struct_rank(int N, char delim, int *rank);
void swap(int *x, int *y);
void q3sort(int *rank, int lo, int hi, int d);
int  cmper(const void *a, const void *b);


int main(int argc, char const *argv[])
{
	char  delim   = argv[1][0];
	//char *id_path = malloc(strlen(argv[2]) * sizeof(char) + 9);	// folder for temporary files
	char *or_path = strdup(argv[3]);
	char *en_path = strdup(argv[4]);
	char *au_path = malloc(strlen(argv[2]) * sizeof(char) + 9);

	/* dealing with newline (\n) as delimiter */
	if (strcmp(argv[1], "\\n") == 0)
		delim = '\n';
	delimiter = delim;

	/* modify correct path of storing temporary and auxiliary files */
	// not going to use tmp folder
	// memset(id_path, 0, strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp"));
	// strcpy(id_path, argv[2]);
	// strcat(id_path, "/bwt.tmp");
	memset(au_path, 0, strlen(argv[4]) * sizeof(char) + strlen(".aux"));
	strcpy(au_path, argv[4]);
	strcat(au_path, ".aux");

	//FILE *idf = fopen(id_path, "w");						  // temporary files to be writen
	FILE *orf = fopen(or_path, "r");					  	  // original csv files to be encoded
	FILE *enf = fopen(en_path, "w");						  // encoded files to be writen
	FILE *auf = fopen(au_path, "wb");						  // auxiliary files to be writen

	int byte_read = 0;
	int f_size = get_f_size(orf);							  // does not take '\0' into account

	buffer = malloc(f_size * sizeof(char) + 1);				  // preserve 1 byte for '\0'

	int *rank = calloc(f_size, sizeof(int));
	memset(buffer, 0, f_size);

	byte_read = fread(buffer, sizeof(char), BUFFER_SIZE, orf);
#ifdef DEBUG
printf("Read file in %dbytes of %dbytes in total\n", 
			byte_read, f_size);
#endif

	/* differentiate delimiters and records */
	int delim_num = struct_rank(f_size, delim, rank);

	buffer[f_size] = '\0';
	/* starting sorting suffix array without delimiters */
	q3sort(rank, delim_num, f_size-1, 0);

	// in order to avoid stack overflow, using pointer instead of array
	ds *delim_stat = malloc(delim_num * sizeof(ds));		// auxiliary files in index folder
	memset(delim_stat, 0, delim_num * sizeof(ds));
	int last_delim_rank = 0;								// delim_stat[0] always last delim

	/**************************************
	* bwt encoding starts here
	***************************************/
	int i, j = 0;
	for (i = 0; i < f_size; i++)
	{
		if (rank[i] > 0)
		{
			fprintf(enf, "%c", buffer[rank[i]-1]);
			if (buffer[rank[i]-1] == delim)
			{
				if (rank[i] == 0)
				{
					last_delim_rank = i;
					continue;
				}
				delim_stat[j].delim_rank = i;
				delim_stat[j].delim_rank_entry = rank[i];
				j++;
			}
		}
		else
		{
			fprintf(enf, "%c", delim);
			if (rank[i] == 0)
			{
				last_delim_rank = i;
				continue;
			}
			delim_stat[j].delim_rank = i;
			delim_stat[j].delim_rank_entry = rank[i];
			j++;
		}
	}

	/* write auxiliary files of delimiter position for future backward search */
	int k;
	qsort(delim_stat, delim_num-1, sizeof(*delim_stat), cmper);
	delim_stat[delim_num-1].delim_rank = last_delim_rank;
	for (k = 0; k < delim_num; k++)
	{
		//printf("%d\n", delim_stat[k].delim_rank);
		fwrite(&delim_stat[k].delim_rank, sizeof(int), 1, auf);
	}

	/* free all mallocated buffers */
	free(delim_stat);
	free(rank);
	free(buffer);

	/* closing all opened files */
	fclose(auf);
	fclose(enf);
	fclose(orf);
	// fclose(idf);

	/* free all malloced arguments */
	free(au_path);
	free(en_path);
	free(or_path);
	// free(id_path);

	return EXIT_SUCCESS;
}

int struct_rank(int N, char delim, int *rank)
{
	int i, j, k;
	i = j = 0;
	k = N - 1;

	while (i < N)
	{
		if (buffer[i] == delim)
			rank[j++] = i;
		else
			rank[k--] = i;
		i++;
	}

	return j;
}

void swap(int *x, int *y)
{
	int tmp;
	tmp = *x;
	*x = *y;
	*y = tmp;
}

void q3sort(int *rank, int lo, int hi, int d)
{
	if (hi <= lo)
		return;

	int lt = lo, gt = hi;
	int v = buffer[rank[lo]+d];
	int i = lo + 1;

	while (i <= gt)
	{
		int t = buffer[rank[i]+d];

		// modification: adding weight to delimiters according to their positions
		if (v == delimiter)
		{
			if (t == delimiter)
			{
				// lower rank gets lower weight
				if (rank[i] < rank[lt])
					swap(&rank[lt++], &rank[i++]);
				else if (rank[i] > rank[lt])
					swap(&rank[i], &rank[gt--]);
				else
					i++;
			}
			else
				swap(&rank[i], &rank[gt--]);				  // v is always smaller than t
		}
		else
		{
			if (t == delimiter)
				swap(&rank[lt++], &rank[i++]);				  // t is always smaller than v
			else
			{
				if (t < v)
					swap(&rank[lt++], &rank[i++]);
				else if (t > v)
					swap(&rank[i], &rank[gt--]);
				else
					i++;	
			}
		}
	}

	q3sort(rank, lo, lt-1, d);
	if (v >= 0)
		q3sort(rank, lt, gt, d+1);
	q3sort(rank, gt+1, hi, d);
}

int cmper(const void *a, const void *b)
{
	const ds *dsa = a, *dsb = b;

	return (dsa->delim_rank_entry < dsb-> delim_rank_entry ?
			-1 : dsa->delim_rank_entry > dsb-> delim_rank_entry);
}

