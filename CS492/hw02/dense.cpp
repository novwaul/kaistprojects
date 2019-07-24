#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctime>
#include <cfloat>
#include <omp.h>
#include <sys/time.h>

#define MAX_THREADS 6

struct dense_mtx
{
    uint32_t nrow; /* Number of rows. */
    uint32_t ncol; /* Number of columns. */
    double *val;   /* Value of this matrix. */
};

struct infomation
{
	struct dense_mtx *A;
	struct dense_mtx *B;
	struct dense_mtx *C;
  uint32_t block_width;
	uint32_t block_num;
	uint32_t block_row;
	uint32_t block_col;
  pthread_mutex_t lock;
};

void *multiply_block (void *aux);
void single (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C);
void pthread (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C, uint32_t p);
void openmp (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C, uint32_t p);

int 
main (int argc, char** argv)
{
	uint32_t n = atoi (argv[1]);
	uint32_t p = argc == 2 ? 1 : atoi (argv[2]);
	bool on = (argc == 2);
	uint32_t total_size = n * n;

  struct dense_mtx A, B, C1, C2, C3;

	
	A.ncol = A.nrow = n;
	A.val = (double *) malloc (total_size * sizeof (double));

	B.ncol = B.nrow = n;
	B.val = (double *) malloc (total_size * sizeof (double));

	C1.ncol = C1.nrow = n;
	C2.ncol = C2.nrow = n;
	C3.ncol = C3.nrow = n;

	srand48 (time (NULL));

	for (int i = 0; i < total_size; i++)
	  {
			A.val[i] = drand48 ();
			B.val[i] = drand48 ();
	  }

	C1.val = (double *) calloc (1, total_size * sizeof (double));
	single (&A, &B, &C1);

	do
	  {
			C2.val = (double *) calloc (1, total_size * sizeof (double));
			C3.val = (double *) calloc (1, total_size * sizeof (double));

			pthread (&A, &B, &C2, p);
			openmp (&A, &B, &C3, p);

			for (int i = 0; i < total_size; i++)
		 		{
			 		if (C1.val[i] == C2.val[i] && C2.val[i] == C3.val[i])
				 		continue;
		 	 		else
		 				{
							std::cout << "Error: invalid result" << std::endl;
							return -1;
		 				}
	 	 		}

			free (C2.val);
			free (C3.val);

			p = (p + 2) - (p % 2);
		 
	  } while (on && p <= MAX_THREADS);

	free (C1.val);
	free (A.val);
	free (B.val);

	return 0;
}

void
single (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C)
{
	struct timeval start, end;
	uint32_t Block = 32;

  std::cout << "single start..." << std::endl;

	gettimeofday (&start, NULL);

	for (uint32_t i = 0; i < C->nrow; i += Block)
		for (uint32_t j = 0; j < C->ncol; j += Block)
			for (uint32_t k = 0; k < C->ncol; k += Block)
				for (uint32_t _i = i; _i < ((i + Block < C->nrow) ? i + Block : C->nrow); _i++)
					for (uint32_t _j = j; _j < ((j + Block < C->ncol) ? j + Block : C->ncol); _j++)
						for (uint32_t _k = k; _k < ((k + Block < C->ncol) ? k + Block : C->ncol); _k++)
							C->val [C->nrow * _i + _j] += A->val [A->nrow * _i + _k] * B->val [B->nrow * _k + _j];
	gettimeofday (&end, NULL);

	std::cout << "single end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

void
pthread (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C, uint32_t p)
{
	struct timeval start, end;
	struct infomation info;
	pthread_t tids[MAX_THREADS];

	std::cout << "pthread with " << p << " threads start..." << std::endl;
	gettimeofday (&start, NULL);
  
	pthread_mutex_init (&info.lock, NULL);
	info.block_row = 0;
	info.block_col = 0;
	info.block_width = 32;
  info.block_num = C->nrow / info.block_width;
	if (C->nrow % info.block_width != 0)
		info.block_num++;
	info.A = A;
	info.B = B;
	info.C = C;

	for (int i = 0; i < p; i++)
		pthread_create (&tids[i], NULL, multiply_block, (void *) &info);

	for (int i = 0; i < p; i++)
		pthread_join (tids[i], NULL);

	gettimeofday (&end, NULL);

	std::cout << "pthread with " << p << " threads end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

void
openmp (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C, uint32_t p)
{
	struct timeval start, end;
	uint32_t Block = 32;

	std::cout << "openmp with " << p << " threads start..." << std::endl;
	gettimeofday (&start, NULL);

#pragma omp parallel default (none) shared(A, B, C, Block) num_threads (p) 
#pragma omp for schedule (guided) collapse (2)
	for (uint32_t i = 0; i < C->nrow; i += Block)
		for (uint32_t j = 0; j < C->ncol; j += Block)
			for (uint32_t k = 0; k < C->ncol; k += Block)
				for (uint32_t _i = i; _i < ((i + Block < C->nrow) ? i + Block : C->nrow); _i++)
					for (uint32_t _j = j; _j < ((j + Block < C->ncol) ? j + Block : C->ncol); _j++)
						for (uint32_t _k = k; _k < ((k + Block < C->ncol) ? k + Block : C->ncol); _k++)
		 					C->val [C->nrow * _i + _j] += A->val [A->nrow * _i + _k] * B->val [B->nrow * _k + _j];

	gettimeofday (&end, NULL);
	std::cout << "openmp with " << p << " threads end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

void *
multiply_block (void *aux)
{
  struct infomation *info = (struct infomation *) aux;
	uint32_t block_row_index, block_col_index, Block, row_start, row_end, col_start, col_end;
	struct dense_mtx *A, *B, *C;

	A = info->A;
	B = info->B;
	C = info->C;
	Block = info->block_width;

	while (true)
		{
			pthread_mutex_lock (&info->lock);
			block_row_index = info->block_row++;
			block_col_index = info->block_col;
			if (info->block_col != info->block_num &&
					info->block_row == info->block_num)
				{
					info->block_row = 0;
					info->block_col++;
				}
			pthread_mutex_unlock (&info->lock);

			if (block_col_index == info->block_num)
				return NULL;
			else
				{
					row_start = block_row_index * Block;
					row_end = row_start + Block;
					col_start = block_col_index * Block;
					col_end = col_start + Block;

					for (uint32_t k = 0; k < C->ncol; k += Block)
						for (uint32_t _i = row_start; _i < ((row_end < C->nrow) ? row_end : C->nrow); _i++)
							for (uint32_t _j = col_start; _j < ((col_end < C->ncol) ? col_end : C->ncol); _j++)
								for (uint32_t _k = k; _k < ((k + Block < C->ncol) ? k + Block : C->ncol); _k++)
									C->val [C->nrow * _i + _j] += A->val [A->nrow * _i + _k] * B->val [B->nrow * _k + _j];
				}
		}
}
