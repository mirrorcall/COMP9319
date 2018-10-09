#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "futil.h"

#define BUFFER_SIZE		1024*1024*42							// maximum input buffer size
#define INFINITY		1.0 / 0.0

static char *buffer;
static char delimiter;

struct _delim_status
{
	int delim_rank;												// sorted based on delim_rank_enty
	int delim_rank_entry;
};

typedef struct _delim_status ds;


int  struct_rank(int N, char delim, int *rank);
int  struct_rank_50(int begin, int end, int *rank, int half);
void swap(int *x, int *y);
void q3sort(int *rank, int lo, int hi, int d);
int  ds_cmper(const void *a, const void *b);
int  delcmp(char *x, int x_idx, char *y, int y_idx);
void merge_files(FILE *rf1, int N1, FILE *rf2, int N2, FILE *wf);


int 
main(int argc, char const *argv[])
{
	char  delim   = argv[1][0];
	char *id1_path = malloc(strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp1"));
	char *id2_path = malloc(strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp2"));
	char *id3_path = malloc(strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp3"));
	char *or_path = strdup(argv[3]);
	char *en_path = strdup(argv[4]);
	char *au_path = malloc(strlen(argv[2]) * sizeof(char) + 9);

	/* dealing with newline (\n) as delimiter */
	if (strcmp(argv[1], "\\n") == 0)
		delim = '\n';
	delimiter = delim;

	/* modify correct path of storing temporary and auxiliary files */
	memset(id1_path, 0, strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp1") + 1);
	strcpy(id1_path, argv[2]);
	strcat(id1_path, "/bwt.tmp1\0");
	
	memset(id2_path, 0, strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp2") + 1);
	strcpy(id2_path, argv[2]);
	strcat(id2_path, "/bwt.tmp2\0");
	
	memset(id3_path, 0, strlen(argv[2]) * sizeof(char) + strlen("/bwt.tmp3") + 1);
	strcpy(id3_path, argv[2]);
	strcat(id3_path, "/bwt.tmp3\0");	
	
	memset(au_path, 0, strlen(argv[4]) * sizeof(char) + strlen(".aux") + 1);
	strcpy(au_path, argv[4]);
	strcat(au_path, ".aux\0");


	FILE *id1fw = fopen(id1_path, "w");						  // temporary files to be writen
	FILE *id2fw = fopen(id2_path, "w");
	FILE *orf = fopen(or_path, "r");					  	  // original csv files to be encoded
	FILE *enf = fopen(en_path, "w");						  // encoded files to be writen
	FILE *auf = fopen(au_path, "wb");						  // auxiliary files to be writen

	int f_size = get_f_size(orf);							  // does not take '\0' into account
	buffer = malloc(f_size * sizeof(char) + 1);				  // preserve 1 byte for '\0'
	memset(buffer, 0, f_size);


	if (f_size <= BUFFER_SIZE)
	{
		fread(buffer, sizeof(char), BUFFER_SIZE, orf);
		buffer[f_size] = '\0';
		int *rank = calloc(f_size, sizeof(int));
		
		/****************************************
		* constructing suffix array starts here
		*****************************************/
		
		int delim_num = struct_rank(f_size, delim, rank);

		/* starting sorting suffix array without delimiters */
		q3sort(rank, delim_num, f_size-1, 0);
		// in order to avoid stack overflow, using pointer instead of array
		ds *delim_stat = malloc(delim_num * sizeof(ds));
		memset(delim_stat, 0, delim_num * sizeof(ds));
		int last_delim_rank = 0;

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
		qsort(delim_stat, delim_num-1, sizeof(*delim_stat), ds_cmper);
		delim_stat[delim_num-1].delim_rank = last_delim_rank;
		for (k = 0; k < delim_num; k++)
			fwrite(&delim_stat[k].delim_rank, sizeof(int), 1, auf);

		free(delim_stat);
		free(rank);
	}
	else		// do the samething but for files over 42M
	{
		fread(buffer, sizeof(char), f_size, orf);
		buffer[f_size] = '\0';

		/****************************************
		* constructing suffix array starts here
		*****************************************/
		int st_hf = f_size / 2;
		int *rank1 = calloc(st_hf, sizeof(int));
		int delim_num1 = struct_rank_50(0, st_hf-1, rank1, 0);
		q3sort(rank1, delim_num1, st_hf-1, 0);
		fwrite(rank1, sizeof(int), st_hf, id1fw);
		// for (int h = 0; h < st_hf; h+=1000)
		// {
		//  	printf("%c\n", buffer[rank1[h]]);
		// }
		free(rank1);
		fclose(id1fw);

		int nd_hf = f_size - (f_size / 2);
		int *rank2 = calloc(nd_hf, sizeof(int));
		int delim_num2 = struct_rank_50(st_hf, f_size-1, rank2, 1);
		q3sort(rank2, delim_num2, nd_hf-1, 0);
		fwrite(rank2, sizeof(int), nd_hf, id2fw);
		// for (int h = 0; h < nd_hf; h+=1000)
		// 	printf("%c\n", buffer[rank2[h]]);
		free(rank2);
		fclose(id2fw);

		FILE *id1fr = fopen(id1_path, "r");
		FILE *id2fr = fopen(id2_path, "r");
		FILE *rankfw = fopen(id3_path, "wb");

		merge_files(id1fr, st_hf, id2fr, nd_hf, rankfw);
		fclose(rankfw);

		/**************************************
		* bwt encoding starts here
		***************************************/
		// rank will be read only by a half to avoid running out of memory
		/* bwt encoding for the first half */
		FILE *rankfr = fopen(id3_path, "rb");
		int *rank_st = malloc(st_hf * sizeof(int));
		int last_delim_rank = 0;
		int delim_num = delim_num1 + delim_num2;
		ds *delim_stat = malloc(delim_num * sizeof(ds));
		fread(rank_st, sizeof(int), st_hf, rankfr);

		int i, j = 0;
		for (i = 0; i < st_hf; i++)
		{
			if (rank_st[i] > 0)
			{
				fprintf(enf, "%c", buffer[rank_st[i]-1]);
				if (buffer[rank_st[i]-1] == delim)
				{
					if (rank_st[i] == 0)
					{
						last_delim_rank = i;
						continue;
					}
					delim_stat[j].delim_rank = i;
					delim_stat[j].delim_rank_entry = rank_st[i];
					j++;
				}
			}
			else
			{
				fprintf(enf, "%c", delim);
				if (rank_st[i] == 0)
				{
					last_delim_rank = i;
					continue;
				}
				delim_stat[j].delim_rank = i;
				delim_stat[j].delim_rank_entry = rank_st[i];
				j++;
			}
		}
		free(rank_st);
		/* 
		 * Write auxiliary files of delimiter position for backward search
		 * All delimiter are guaranteed to appear in the first half of rank
		 */
		int k;
		qsort(delim_stat, delim_num-1, sizeof(*delim_stat), ds_cmper);
		delim_stat[delim_num-1].delim_rank = last_delim_rank;
		for (k = 0; k < delim_num; k++)
			fwrite(&delim_stat[k].delim_rank, sizeof(int), 1, auf);


		/* bwt encoding for the second half */
		int *rank_nd = malloc(nd_hf * sizeof(int));
		fread(rank_nd, sizeof(int), nd_hf, rankfr);

		for (i = 0; i < nd_hf; i++)
		{
			if (rank_nd[i] > 0)
				fprintf(enf, "%c", buffer[rank_nd[i]-1]);
			else
				fprintf(enf, "%c", delim);
		}
		free(rank_nd);
		free(delim_stat);

		fclose(rankfr);
		fclose(id2fr);
		fclose(id1fr);
	}

	/* free all mallocated buffers */
	free(buffer);

	/* closing all opened files */
	fclose(auf);
	fclose(enf);
	fclose(orf);

	/* free all malloced arguments */
	free(au_path);
	free(en_path);
	free(or_path);
	
	/* very last thing */
	remove(id3_path);
	remove(id2_path);
	remove(id1_path);
	free(id3_path);
	free(id2_path);
	free(id1_path);

	return EXIT_SUCCESS;
}

int 
struct_rank(int N, char delim, int *rank)
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

int
struct_rank_50(int begin, int end, int *rank, int half)
{
	int i, j, k;

	if (half == 0)		// first half
	{
		i = j = 0;
		k = end;
	}
	else 				// second half
	{
		i = begin;
		j = 0;
		k = end - begin;
	}

	while (i <= end)
	{
		if (buffer[i] == delimiter)
			rank[j++] = i;
		else
			rank[k--] = i;
		i++;
	}

	return j;
}

void 
swap(int *x, int *y)
{
	int tmp;
	tmp = *x;
	*x = *y;
	*y = tmp;
}

void 
q3sort(int *rank, int lo, int hi, int d)
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

int
delcmp(char *x, int x_idx, char *y, int y_idx)
{
	while (*x && (*x == *y))
	{
		x++;
		y++;

		if (*x == delimiter)
			return (x_idx - y_idx);
	}

	if (*x == delimiter)
	{
		if (*y == delimiter)
			return (x_idx - y_idx);
		else
			return -1;
	}
	else
	{
		if (*y == delimiter)
			return (x_idx < y_idx);
		else
			return (*x - *y);
	}

	return strcmp(x, y);
}

void 
merge_files(FILE *rf1, int N1, FILE *rf2, int N2, FILE *wf)
{
	int num1, num2;
	int i, j;
	i = j = 0;
	fread(&num1, sizeof(int), 1, rf1);
	fread(&num2, sizeof(int), 1, rf2);

	while ((i < N1) && (j < N2))
	{
		if (buffer[num1] == delimiter)
		{
			fwrite(&num1, sizeof(int), 1, wf);
			fread(&num1, sizeof(int), 1, rf1);
			i++;
			continue;
		}
		if (buffer[num2] == delimiter)
		{
			fwrite(&num2, sizeof(int), 1, wf);
			fread(&num2, sizeof(int), 1, rf2);
			j++;
			continue;
		}

		if (buffer[num1] < buffer[num2])
		{
			fwrite(&num1, sizeof(int), 1, wf);
			i++;
			fread(&num1, sizeof(int), 1, rf1);
		}
		else if (buffer[num1] > buffer[num2])
		{
			fwrite(&num2, sizeof(int), 1, wf);
			j++;
			fread(&num2, sizeof(int), 1, rf2);
		}
		else
		{
			int cmp = delcmp(&buffer[num1], num1, &buffer[num2], num2);
			if (cmp < 0)
			{
				fwrite(&num1, sizeof(int), 1, wf);
				i++;
				fread(&num1, sizeof(int), 1, rf1);
			}
			else
			{					
				fwrite(&num2, sizeof(int), 1, wf);
				j++;
				fread(&num2, sizeof(int), 1, rf2);
			}
		}
	}

	while (i < N1)
	{
		fwrite(&num1, sizeof(int), 1, wf);		
		fread(&num1, sizeof(int), 1, rf1);
		i++;
	}

	while (j < N2)
	{
		fwrite(&num2, sizeof(int), 1, wf);		
		fread(&num2, sizeof(int), 1, rf2);
		j++;
	}
}

int 
ds_cmper(const void *a, const void *b)
{
	const ds *dsa = a, *dsb = b;

	return (dsa->delim_rank_entry < dsb-> delim_rank_entry ?
			-1 : dsa->delim_rank_entry > dsb-> delim_rank_entry);
}

