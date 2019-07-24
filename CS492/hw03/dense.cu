#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctime>
#include <cfloat>
#include <omp.h>
#include <sys/time.h>
#include <cmath>

#define MAX_THREADS 6

struct dense_mtx
{
    uint32_t nrow; /* Number of rows */
    uint32_t ncol; /* Number of columns */
    double *val;   /* Value of this matrix. */
};

void single (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C);
void cuda (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C);

int 
main (int argc, char** argv)
{
	uint32_t n = atoi (argv[1]);
	uint32_t total_size = n * n;

  struct dense_mtx A, B, C1, C2;

	
	A.ncol = A.nrow = n;
	A.val = (double *) malloc (total_size * sizeof (double));

	B.ncol = B.nrow = n;
	B.val = (double *) malloc (total_size * sizeof (double));

	C1.ncol = C1.nrow = n;
	C2.ncol = C2.nrow = n;

	srand48 (time (NULL));

	for (int i = 0; i < total_size; i++)
	  {
			A.val[i] = drand48 ();
			B.val[i] = drand48 ();
		}
  
	C1.val = (double *) calloc (1, total_size * sizeof (double));
	single (&A, &B, &C1);
  
	C2.val = (double *) calloc (1, total_size * sizeof (double));
	cuda (&A, &B, &C2);
  
	double L2norm = 0, diff;
	for (int i = 0; i < total_size; i++)
		{
			diff = (C1.val[i] - C2.val[i]);
			L2norm += diff * diff;
	 	}

	std::cout << "L2norm: " << sqrt (L2norm) << std::endl;

	free (C1.val);
	free (C2.val);
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
				for (uint32_t _i = i; _i < (((i + Block) < C->nrow) ? i + Block : C->nrow); _i++)
					for (uint32_t _j = j; _j < (((j + Block) < C->ncol) ? j + Block : C->ncol); _j++)
						for (uint32_t _k = k; _k < (((k + Block) < C->ncol) ? k + Block : C->ncol); _k++)
							C->val [C->nrow * _i + _j] += A->val [A->nrow * _i + _k] * B->val [B->nrow * _k + _j];
							
	gettimeofday (&end, NULL);

	std::cout << "single end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

	__global__ void
matrix_multiply (double *A, double *B, double *C, uint32_t n)
{
	uint32_t Block = 16;

  __shared__ double TileA[16][16];
	__shared__ double TileB[16][16];

	uint32_t tx = threadIdx.x;
	uint32_t ty = threadIdx.y;
	uint32_t Row = Block * blockIdx.y + ty;
	uint32_t Col = Block * blockIdx.x + tx;

	double val = 0;

	for (uint32_t i = 0; i < (n + Block - 1) / Block; i++)
		{
			/* Fetch data to shared memeory. */
			if ((i * Block + tx) < n && Row < n)
				TileA [ty][tx] = A [Row * n + i * Block + tx];
			else
				TileA [ty][tx] = 0;

			if ((i * Block + ty) < n && Col < n)
				TileB [ty][tx] = B [(i * Block + ty) * n + Col];
			else
				TileB [ty][tx] = 0;

			__syncthreads ();
			for (uint32_t k = 0; k < Block; k++)
				val += TileA [ty][k] * TileB [k][tx];
			__syncthreads ();
		}
	if (Row < n && Col < n)
	  C [Row * n + Col] = val;
}

void
cuda (struct dense_mtx *A, struct dense_mtx *B, struct dense_mtx *C)
{
	struct timeval start, end;
	uint32_t Block = 16;
	uint32_t size;
	double *A_val, *B_val, *C_val;

  std::cout << "cuda start..." << std::endl;

	gettimeofday (&start, NULL);

  dim3 dimGrid ((A->nrow + Block - 1) / Block, (A->ncol + Block - 1) / Block, 1);
	dim3 dimBlock (Block, Block, 1);

	size = A->nrow * A->ncol * sizeof (double);
	
	cudaMalloc (&A_val, size);
	cudaMalloc (&B_val, size);
	cudaMalloc (&C_val, size);

	cudaMemcpy (A_val, A->val, size, cudaMemcpyHostToDevice);
	cudaMemcpy (B_val, B->val, size, cudaMemcpyHostToDevice);

	matrix_multiply<<<dimGrid, dimBlock>>>(A_val, B_val, C_val, A->nrow);

	cudaMemcpy (C->val, C_val, size, cudaMemcpyDeviceToHost);

	cudaFree (A_val);
	cudaFree (B_val);
	cudaFree (C_val);

	gettimeofday (&end, NULL);

	std::cout << "cuda end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}
