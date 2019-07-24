#include "mmreader.hpp"
#include <time.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <omp.h>
#include <pthread.h>
#include <list>
#include <cmath>

#define WARP 32
#define MAXBLOCK 65535

void validate (struct dense_mtx *C1, struct dense_mtx *C2);

bool
SCsrMatrixfromFile(struct sparse_mtx *A, const char* filePath)
{
    // Check that the file format is matrix market; the only format we can read right now
    // This is not a complete solution, and fails for directories with file names etc...
    // TODO: Should we use boost filesystem?
    std::string strPath( filePath );
    if( strPath.find_last_of( '.' ) != std::string::npos )
    {
        std::string ext = strPath.substr( strPath.find_last_of( '.' ) + 1 );
        if( ext != "mtx" )
        {
            std::cout << "Reading file name error" << std::endl;
            return false;
        }
    }
    else
        return false;

    // Read data from a file on disk into buffers
    // Data is read natively as COO format with the reader
    MatrixMarketReader mm_reader;
    if( mm_reader.MMReadFormat(filePath) )
        return false;

    // JPA: Shouldn't that just be an assertion check? It seems to me that
    // the user have to call clsparseHeaderfromFile before calling this function,
    // otherwise the whole pCsrMatrix will be broken;
    A->nrow = mm_reader.GetNumRows( );
    A->ncol = mm_reader.GetNumCols( );
    A->nnze = mm_reader.GetNumNonZeroes( );

    A->row = (int32_t *)malloc((A->nrow + 1) * sizeof(int32_t));
    A->val = (float *)malloc(A->nnze * sizeof(float));
    A->col = (int32_t *)malloc(A->nnze * sizeof(int32_t));

    if(A->row == NULL || A->col == NULL || A->val == NULL)
    {
        if(A->row == NULL)
            free((void *)A->row);
        if(A->col == NULL)
            free((void *)A->col);
        if(A->val == NULL)
            free((void *)A->val);
        return false;
    }

    //  The following section of code converts the sparse format from COO to CSR
    Coordinate* coords = mm_reader.GetUnsymCoordinates( );

    std::sort( coords, coords + A->nnze, CoordinateCompare );

    int32_t current_row = 1;

    A->row[ 0 ] = 0;

    for (int32_t i = 0; i < A->nnze; i++)
    {
        A->col[ i ] = coords[ i ].y;
        A->val[ i ] = coords[ i ].val;

        while( coords[ i ].x >= current_row )
            A->row[ current_row++ ] = i;
    }

    A->row[ current_row ] = A->nnze;

    while( current_row <= A->nrow )
        A->row[ current_row++ ] = A->nnze;

    return true;
}

void multiply_single(struct sparse_mtx *A, struct dense_mtx *B, struct dense_mtx *C)
{
    C->nrow = A->nrow;
    C->ncol = B->ncol;
    C->val = (float *)calloc(1, C->nrow * C->ncol * sizeof(float));

    if(C->val == NULL)
        return;
    
    for(int32_t i = 0; i < A->nrow; i++)
    {
        int32_t A_col_start = A->row[i];
        int32_t A_col_stop = A->row[i + 1];
        
        for(int32_t j = A_col_start; j < A_col_stop; j++)
        {
            int32_t B_row = A->col[j];

            for(int32_t k = 0; k < B->ncol; k++)
                C->val[i * C->ncol + k] += A->val[j] * B->val[B_row * B->ncol + k];
        }
    }
}

__global__ void
multiply_matrix (int32_t *A_row, int32_t *A_col, float *A_val, float *B, float *C, uint32_t n, uint32_t m, int32_t ofs)
{
	volatile __shared__ float vals [WARP];
	int32_t col_start;
	int32_t col_stop;
	int32_t tx = threadIdx.x;
	int32_t Row = ofs + blockIdx.y;
	int32_t Col = blockIdx.x;

	vals [tx] = 0.0f;

	col_start = A_row [Row];
	col_stop = A_row [Row + 1];

	for (int32_t j = col_start + tx; col_start <= j && j < col_stop; j += WARP)
		{
			float coef = A_val [j];
			float bval = B [A_col [j] * m + Col];
			vals [tx] += bval * coef;
		}

	int32_t size = WARP;
	while (size > 1)
		{
			size /= 2;
			if (tx < size)
				{
					vals [tx] += vals [tx + size];
				}
			else;
		}
	
	if (tx == 0)
		{
			C [Row * m + Col] = vals [0];
		}

	else;
}

void cuda (struct sparse_mtx *A, struct dense_mtx *B, struct dense_mtx *C)
{
	C->nrow = A->nrow;
	C->ncol = B->ncol;
	/* Allocate C value memory */
	C->val = (float *) calloc (1, sizeof (float) * C->nrow * C->ncol);
	
	/* Allocate GPU memory */
	int32_t *Ad_row, *Ad_col;
	float *Ad_val, *Bd, *Cd;
	if (cudaMalloc (&Ad_row, sizeof (int32_t) * (A->nrow + 1)) != cudaSuccess
			|| cudaMalloc (&Ad_col, sizeof (int32_t) * A->nnze) != cudaSuccess
			|| cudaMalloc (&Ad_val, sizeof (float) * A->nnze) != cudaSuccess
			|| cudaMalloc (&Bd, sizeof (float) * B->nrow * B->ncol) != cudaSuccess
			|| cudaMalloc (&Cd, sizeof (float) * C->nrow * C->ncol) != cudaSuccess )
	{
		printf ("Memory Allocation Error!\n");
		return;
	}

	/* Send data. */
	if (cudaMemcpy (Ad_row, A->row, sizeof (int32_t) * (A->nrow + 1), cudaMemcpyHostToDevice) != cudaSuccess
			|| cudaMemcpy (Ad_col, A->col, sizeof (int32_t) * A->nnze, cudaMemcpyHostToDevice) != cudaSuccess
			|| cudaMemcpy (Ad_val, A->val, sizeof (float) * A->nnze, cudaMemcpyHostToDevice) != cudaSuccess
			|| cudaMemcpy (Bd, B->val, sizeof (float) * B->nrow * B->ncol, cudaMemcpyHostToDevice) != cudaSuccess)
	{
		printf ("Memcpy Error: Host to Device\n");
		return;
	}

	int32_t ofs = 0;
	int32_t size = C->nrow;
	while (size > MAXBLOCK)
		{
			dim3 dimGrid (C->ncol, MAXBLOCK, 1);
			
			multiply_matrix<<<dimGrid, WARP>>> (Ad_row, Ad_col, Ad_val, Bd, Cd, C->nrow, C->ncol, ofs);

			if (cudaSuccess != cudaGetLastError ())
				{
					printf ("Kernel Error\n");
					return;
				}

			size -= MAXBLOCK;
			ofs += MAXBLOCK;
		}

	dim3 dimGrid (C->ncol, size, 1);
	multiply_matrix<<<dimGrid, WARP>>> (Ad_row, Ad_col, Ad_val, Bd, Cd, C->nrow, C->ncol, ofs);

	if (cudaSuccess != cudaGetLastError ())
		{
			printf ("Kernel Error\n");
			return;
		}

	/* Get data. */
	if (cudaMemcpy (C->val, Cd, sizeof (float) * C->nrow * C->ncol, cudaMemcpyDeviceToHost) != cudaSuccess)
		{
			printf ("Memcpy Error: Device to Host\n");
			return;
		}

	/* Deallocate GPU memory. */
	cudaFree (Ad_row);
	cudaFree (Ad_col);
	cudaFree (Ad_val);
	cudaFree (Bd);
	cudaFree (Cd);
}

uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

int main(int argc, char **argv)
{
    struct sparse_mtx A;
    if(!SCsrMatrixfromFile(&A, argv[1]))
    {
        std::cout << "read failed." << std::endl;
        return 0;
    }
    std::cout << "Matrix: " << argv[1] << std::endl;

    struct dense_mtx B;
    B.nrow = A.ncol;
    B.ncol = atoi(argv[2]);

    if((int) B.ncol < 0)
    {
        free(A.row);
        free(A.col);
        free(A.val);
        std::cerr << "Invalid argument for the number of columns of B." << std::endl;
    }

    B.val = (float *)malloc(sizeof(float) * B.nrow * B.ncol);

    srand((unsigned int)time(NULL));
    for(int i = 0; i < B.nrow; i++)
    {
        for(int j = 0; j < B.ncol; j++)
        {
            B.val[B.ncol * i + j] = ((float)rand()/(float)(RAND_MAX)) * ((rand() % 2) ? 1.0f : -1.0f);
        }
    }

    struct dense_mtx C1, C2;
    C1.val = C2.val = NULL;
		
	struct timeval start, end;

		/* Single. */
		std::cout << "single start..." << std::endl;
    gettimeofday (&start, NULL);
    multiply_single(&A, &B, &C1);
    gettimeofday (&end, NULL);
    std::cout << "single end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;

		/* GPU. */
		std::cout << "cuda start..." << std::endl;
    gettimeofday (&start, NULL);
    cuda (&A, &B, &C2);
    gettimeofday (&end, NULL);
    std::cout << "cuda end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;

		validate (&C1, &C2);

    free(A.row);
    free(A.col);
    free(A.val);
    free(B.val);
    if (C1.val != NULL)
      free(C1.val);
 		if (C2.val != NULL)
      free(C2.val); 
   
    return 0;
}

void
validate (struct dense_mtx *C1, struct dense_mtx *C2)
{
	float diff;
	float L2norm = 0.000000f;
	for (int32_t i = 0; i < C1->nrow; i++)
		for (int32_t j = 0; j < C1->ncol; j++)
			{
				float c1val = C1->val [i * C1->ncol + j];
				float c2val = C2->val [i * C2->ncol + j];
				diff = c1val - c2val;
				while (c1val >= 1 || c1val <= -1 || c2val >= 1 || c2val <= -1)
					{
						c1val /= 10;
						c2val /= 10;
						diff /= 10;
					}
				L2norm += diff * diff;
			}
	printf ("L2norm: %f\n", sqrt (L2norm));
}
