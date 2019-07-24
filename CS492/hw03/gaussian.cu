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

struct info
{
		double val;
		uint32_t pos;
};

void single (struct dense_mtx *A, struct dense_mtx *b);
void L2norm (struct dense_mtx *A, struct dense_mtx *b, struct dense_mtx *_b);
void replicate (struct dense_mtx *A, struct dense_mtx *b, struct dense_mtx *_A, struct dense_mtx *_b);
void cuda (struct dense_mtx *A, struct dense_mtx *b);

int 
main (int argc, char** argv)
{
	uint32_t n = atoi (argv[1]);
	uint32_t total_size = n * (n + 1);

  struct dense_mtx A, A1, A2, b, b1, b2;

	A.nrow = n;
	A.ncol = n;
	b.nrow = n;
	b.ncol = 1;
	A.val = (double *) malloc (total_size * sizeof (double));
	b.val = (double *) malloc (n * sizeof (double));

	A1.nrow = A2.nrow = n;
	A1.ncol = A2.ncol = n;
	b1.nrow = b2.nrow = n;
	b1.ncol = b2.ncol = 1;

	srand48 (time (NULL));

	for (uint32_t i = 0; i < total_size; i++)
		A.val [i] = drand48 ();
	  
	for (uint32_t i = 0; i < n; i++)
		b.val [i] = drand48 ();

	A1.val = (double *) malloc (total_size * sizeof (double));
	b1.val = (double *) malloc (n * sizeof (double));

	replicate (&A, &b, &A1, &b1);

	single (&A1, &b1);
	L2norm (&A, &b, &b1);

	free (A1.val);
	free (b1.val);

	A2.val = (double *) malloc (total_size * sizeof (double));
	b2.val = (double *) malloc (n * sizeof (double));

	replicate (&A, &b, &A2, &b2);
  cuda (&A2, &b2);

	L2norm (&A, &b, &b2);

	free (A2.val);
	free (b2.val);

	free (A.val);
	free (b.val);

	return 0;
}

__global__ void
gaussian (double *A, double *b, uint32_t n, uint32_t cur_pos, double* m_ptr)
{
	uint32_t Block = 16;
  uint32_t tx = threadIdx.x;
	uint32_t ty = threadIdx.y;
	uint32_t Row = Block * blockIdx.y + ty + 1 + cur_pos;
	uint32_t Col = Block * blockIdx.x + tx + 1 + cur_pos;
	
	__shared__ double subA [16][16];
	__shared__ double pivot_Row [16];
	__shared__ double pivot_Col [16];
	__shared__ double m;

	/* Get m. */
	if (tx == ty == 0)
		m = *m_ptr;

	__syncthreads ();

	/* Get subA. */	
	if (Row < n && Col < n)
		{
			subA [ty][tx] = A [Row * n + Col];
		}
	else if (Row < n && Col == n)
	  {
			subA [ty][tx] = b [Row];
	  }
	else
		{
			subA [ty][tx] = 0;
		}
	
	/* Get pivots */
	if (ty == 0)
		{
			uint32_t pRow = Block * blockIdx.y + tx + 1 + cur_pos;
			if (pRow < n)
				{
					pivot_Col [tx] = A [pRow * n + cur_pos];
				}
			else
				{
					pivot_Col [tx] = 0;
				}

			if (Col < n)
				{
					pivot_Row [tx] = A [cur_pos * n + Col];
				}
			else if (Col == n)
				{
					pivot_Row [tx] = b [cur_pos];
				}
			else
				{
					pivot_Row [tx] = 0;
				}
		}

	__syncthreads ();

	/* Do gaussian. */
	subA [ty][tx] = subA [ty][tx] - (pivot_Col [ty] / m) * pivot_Row [tx];

	/* Update real matrix. */
	if (Row < n && Col < n)
		{
			A [Row * n + Col] = subA [ty][tx];
		}
	else if (Row < n && Col == n)
	  {
			b [Row] = subA [ty][tx];
	  }
}

__global__ void
swap (double *A, double *b, uint32_t n, uint32_t cur_pos, uint32_t *max_pos_ptr)
{
	uint32_t Block = 256;
	uint32_t idx = threadIdx.x + Block * blockIdx.x + cur_pos;
	__shared__ uint32_t max_pos;

	if (threadIdx.x == 0)
		max_pos = *max_pos_ptr;

	__syncthreads ();
	double temp;
	if (idx < n)
		{
			temp = A [cur_pos * n + idx];
			A [cur_pos * n + idx] = A [max_pos * n + idx];
			A [max_pos * n + idx] = temp;
		}
	else if (idx == n)
		{
			temp = b [cur_pos];
			b [cur_pos] = b [max_pos];
			b [max_pos] = temp;
		}
}

__global__ void
findmaxloc (double *A, uint32_t n, uint32_t cur_pos, uint32_t *max_Row, double *m, struct info *maxBuf)
{

	extern __shared__ struct info pivotCol [];
	
	uint32_t tx = threadIdx.x;
	uint32_t Block = 256;
	uint32_t threshold = 3000;
	uint32_t loop_count, i = 0, j = 0;
	uint32_t remain = n - cur_pos;
	uint32_t size;

	/* Find local maximum values & positions */
	while (remain > 0)
		{
			/* Check if remain is larger than threshold. */
			if (remain > threshold)
				size = threshold;
			else
				size = remain;
		
			/* Calculate loop count. */
			loop_count = (size + 2 * Block - 1) / (2 * Block);

			/* Fetch data to shared memory. */
			while (loop_count > i)
				{
					uint32_t A_index = (cur_pos + i * Block * 2 + tx * 2 + threshold * j);
					uint32_t P_index = i * Block * 2 + tx * 2;
			
					if (A_index < cur_pos + j * threshold + size)
						{
			    		pivotCol [P_index].val = A [A_index * n + cur_pos];
			    		pivotCol [P_index].pos = A_index;
						}

					if ((A_index + 1) < cur_pos + j * threshold + size)
						{
							pivotCol [P_index + 1].val = A [(A_index + 1) * n + cur_pos];
							pivotCol [P_index + 1].pos = A_index + 1;
						}
					
					i++;
				}

			__syncthreads ();

			uint32_t temp = size;
			/* Find max location. */
			while (size > 1)
				{
					i = 0;
					while (loop_count > i)
						{	
							uint32_t P_index = i * Block * 2 + tx * 2;
							uint32_t pos;
							double val;
							if (P_index < size && P_index + 1 < size)
								{
									if (abs (pivotCol [P_index].val) > abs (pivotCol [P_index + 1].val))
										{
											pos = pivotCol [P_index].pos;
											val = pivotCol [P_index].val;
										}
									else
										{
											pos = pivotCol [P_index + 1].pos;
											val = pivotCol [P_index + 1].val;
										}
								}
							else if (P_index < size)
								{
									pos = pivotCol [P_index].pos;
									val = pivotCol [P_index].val;
								}
							__syncthreads ();
							if (P_index < size)
								{
									pivotCol [P_index / 2].pos = pos;
									pivotCol [P_index / 2].val = val;
								}
							__syncthreads ();
							i++;
						}
					size = (size + 1) / 2;
				}

			/* Store local maximum value. */
			if (tx == 0)
				{
					maxBuf [j].val = pivotCol [0].val;
					maxBuf [j].pos = pivotCol [0].pos;
				}

		  __syncthreads ();
			/* Advance. */
			j++;
			remain -= temp;

		}

	/* Find Global maximum value & its position. */

	loop_count = (j + 2 * Block - 1) / (2 * Block);
	i = 0;
	while (loop_count > i)
		{
			uint32_t P_index = i * Block * 2 + tx * 2;
			
			if (P_index < j)
				{
		  		pivotCol [P_index].val = maxBuf [P_index].val;
		  		pivotCol [P_index].pos = maxBuf [P_index].pos;
				}

			if ((P_index + 1) < j)
				{
					pivotCol [P_index + 1].val = maxBuf [P_index].val;
					pivotCol [P_index + 1].pos = maxBuf [P_index].pos;
				}
		
			i++;
		}

	
	size = j;
	while (size > 1)
		{
			i = 0;
			while (loop_count > i)
				{	
					uint32_t P_index = i * Block * 2 + tx * 2;
					uint32_t pos;
					double val;
					if (P_index < size && P_index + 1 < size)
						{
							if (abs (pivotCol [P_index].val) > abs (pivotCol [P_index + 1].val))
								{
									pos = pivotCol [P_index].pos;
									val = pivotCol [P_index].val;
								}
							else
								{
									pos = pivotCol [P_index + 1].pos;
									val = pivotCol [P_index + 1].val;
								}
						}
					else if (P_index < size)
						{
							pos = pivotCol [P_index].pos;
							val = pivotCol [P_index].val;
						}
					__syncthreads ();
					if (P_index < size)
						{
							pivotCol [P_index / 2].pos = pos;
							pivotCol [P_index / 2].val = val;
						}
					__syncthreads ();
					i++;
				}
			size = (size + 1) / 2;
		}

	if (tx == 0)
		{
			*m = pivotCol [0].val;
			*max_Row = pivotCol [0].pos;
		}
}

void
cuda (struct dense_mtx *A, struct dense_mtx *b)
{
	struct timeval start, end;
	uint32_t Block = 16;
  double *A_val, *b_val;

	std::cout << "cuda start..." << std::endl;
	gettimeofday (&start, NULL);

	uint32_t A_size = A->nrow * A->ncol * sizeof (double);
	uint32_t b_size = b->nrow * b->ncol * sizeof (double);

	cudaMalloc (&A_val, A_size);
	cudaMalloc (&b_val, b_size);

	cudaMemcpy (A_val, A->val, A_size, cudaMemcpyHostToDevice);
	cudaMemcpy (b_val, b->val, b_size, cudaMemcpyHostToDevice);

	uint32_t cur_Col, cur_Row;

	uint32_t *max_Row;
	double *m;

	cudaMalloc (&max_Row, sizeof (double));
	cudaMalloc (&m, sizeof (double));

	struct info *maxBuf;
	
	cudaMalloc (&maxBuf, ((A->nrow + 3000 - 1) / 3000) * sizeof (struct info));

	cur_Col = cur_Row = 0;
	while (cur_Row < (A->nrow - 1))
		{
	    /* Find max location. */
			uint32_t threshold = 3000;
			uint32_t cur = A->nrow - cur_Row;
			uint32_t elements;
			if (cur > threshold)
				elements = threshold;
			else
				elements = cur;
			findmaxloc<<<1, 256, elements * sizeof (struct info) >>>(A_val, A->nrow, cur_Row, max_Row, m, maxBuf);
			
			/* Swap. */
			swap<<<((A->ncol + 1 - cur_Col) + 256 - 1) / 256, 256>>>(A_val, b_val, A->nrow, cur_Row, max_Row);
		 
			/* Gaussian. */
			dim3 dimGrid (((A->nrow - cur_Row) + Block - 1) / Block, ((A->ncol - cur_Col - 1) + Block - 1) / Block, 1);
			dim3 dimBlock (Block, Block, 1);
  		gaussian<<<dimGrid, dimBlock>>>(A_val, b_val, A->nrow, cur_Row, m);

			/* Advance */
			cur_Row++;
			cur_Col++;
		}

	/* Get data. */
	cudaMemcpy (A->val, A_val, A_size, cudaMemcpyDeviceToHost);
	cudaMemcpy (b->val, b_val, b_size, cudaMemcpyDeviceToHost);

	cudaFree (A_val);
	cudaFree (b_val);
	cudaFree (m);
	cudaFree (max_Row);
	cudaFree (maxBuf);

  /* Back substitution. */
	for (int32_t i = A->nrow - 1; i >= 0; i--)
		{
			b->val [i] /= A->val [A->ncol * i + i];
			for (uint32_t j = i + 1; j < A->ncol; j++)
				b->val [i] -= b->val [j] * A->val [A->ncol * i + j] / A->val [A->ncol * i + i];
		}

	gettimeofday (&end, NULL);
	std::cout << "cuda end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

void
single (struct dense_mtx *A, struct dense_mtx *b)
{
	struct timeval start, end;
	uint32_t mrow, srow, scol;
	double max,temp, m;

  std::cout << "single start..." << std::endl;

	gettimeofday (&start, NULL);

	
	srow = scol = mrow = 0;

	while (srow < (A->nrow - 1))
		{
			/* Find max location. */
			max = 0;
			for (uint32_t i = srow; i < A->nrow; i++)
				{
					double val = A->val [A->ncol * i + scol];
					double tval, tmax;

					tval = val > 0 ? val : -val;
					tmax = max > 0 ? max : -max;

					if (tval > tmax)
						{
							max = val;
							mrow = i;
						}
				}

			/* Swap. */
		  for (uint32_t j = scol; j < A->ncol; j++)
			  {
			    temp = A->val [A->ncol * srow + j];
		 	    A->val [A->ncol * srow + j] = A->val [A->ncol * mrow + j];
				  A->val [A->ncol * mrow + j] = temp;
			  }
			temp = b->val [srow];
			b->val [srow] = b->val [mrow];
			b->val [mrow] = temp;

			/* Gaussian elimination. */	
			for (uint32_t i = srow + 1; i < A->nrow; i++)
				{
					m = A->val [A->ncol * i + scol] / A->val [A->ncol * srow + scol];
					for (uint32_t j = scol; j < A->ncol; j++)
						A->val [A->ncol * i + j] -= m * A->val [A->ncol * srow + j];
					b->val [i] -= m * b->val [srow];
				}

			srow++;
			scol++;
		}

	/* Back substitution. */
	for (int32_t i = A->nrow - 1; i >= 0; i--)
		{
			b->val [i] /= A->val [A->ncol * i + i];
			for (uint32_t j = i + 1; j < A->ncol; j++)
				b->val [i] -= b->val [j] * A->val [A->ncol * i + j] / A->val [A->ncol * i + i];
		}

	gettimeofday (&end, NULL);

	std::cout << "single end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

void
replicate (struct dense_mtx *A, struct dense_mtx *b, struct dense_mtx *_A, struct dense_mtx *_b)
{
	for (uint32_t i = 0; i < A->nrow; i++)
		for (uint32_t j = 0; j < A->ncol; j++)
			_A->val [_A->ncol * i + j] = A->val [A->ncol * i + j];

	for (uint32_t i = 0; i < b->nrow; i++)
		_b->val [i] = b->val [i];
}

void
L2norm (struct dense_mtx *A, struct dense_mtx *b, struct dense_mtx *_b)
{
  double diffsq, temp;

	diffsq = 0;

	for (uint32_t i = 0; i < A->nrow; i++)
		{
			temp = 0;
			for (uint32_t j = 0; j < A->ncol; j++)
				temp += A->val [A->ncol * i + j] * _b->val [j];
			diffsq += (b->val [i] - temp) * (b->val [i] - temp);
		}

	std::cout << "L2-norm: " << sqrt (diffsq) << std::endl;
}

