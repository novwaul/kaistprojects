#include "mmreader.hpp"
#include <time.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <omp.h>
#include <pthread.h>

#define MAX_THREADS 6

struct infomation
{
	uint32_t i;
	struct sparse_mtx *A;
	struct dense_mtx *B;
	struct dense_mtx *C;
	pthread_mutex_t lock;
};

bool validate (struct dense_mtx *C1, struct dense_mtx *C2);

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
    C->val = (float *)malloc(C->nrow * C->ncol * sizeof(float));

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

void *multiply (void *aux)
{
		struct infomation *info = (struct infomation *) aux;
		struct dense_mtx *B, *C;
		struct sparse_mtx *A;
		uint32_t i, j, Block = 256;
   
		A = info->A;
		B = info->B;
		C = info->C;

		while (true)
			{
	 		  pthread_mutex_lock (&info->lock);
				i = info->i;
				info->i += Block;
				pthread_mutex_unlock (&info->lock);

				if (i >= A->nrow)
					return NULL;

			  for(int32_t _i = i; _i < (i + Block > A->nrow ? A->nrow : i + Block); _i++)
    			{
        		int32_t A_col_start = A->row[_i];
       			int32_t A_col_stop = A->row[_i + 1];
        
        		for(int32_t j = A_col_start; j < A_col_stop; j++)
        			{
            		int32_t B_row = A->col[j];

            		for (int32_t k = 0; k < B->ncol; k++)
                	C->val[_i * C->ncol + k] += A->val[j] * B->val[B_row * B->ncol + k];
        			}
    			}
	
			}
}

void multiply_pthread(struct sparse_mtx *A, struct dense_mtx *B, struct dense_mtx *C, uint32_t p)
{
	C->nrow = A->nrow;
	C->ncol = B->ncol;
	C->val = (float *) malloc (C->nrow * C->ncol * sizeof (float));

	if (C->val == NULL)
		return;

	struct infomation info;
	pthread_t tids [MAX_THREADS];

	info.A = A;
	info.B = B;
	info.C = C;
	info.i = 0;
	pthread_mutex_init (&info.lock, NULL);

	for (uint32_t i = 0; i < p; i++)
		pthread_create (&tids [i], NULL, multiply, (void *) &info);

	for (uint32_t i = 0; i < p; i++)
		pthread_join (tids [i], NULL);
}

void multiply_openmp(struct sparse_mtx *A, struct dense_mtx *B, struct dense_mtx *C, uint32_t p)
{
		C->nrow = A->nrow;
		C->ncol = B->ncol;
		C->val = (float *) malloc (C->nrow * C->ncol * sizeof (float));

		if (C->val == NULL)
			return;

#pragma omp parallel for default (none) shared (A, B, C) schedule (guided) num_threads (p)
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
    if(B.ncol < 0)
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

    struct dense_mtx C1, C2, C3;
    C1.val = NULL;
    C2.val = NULL;
		C3.val = NULL;

		struct timeval start, end;

		std::cout << "single start..." << std::endl;
    gettimeofday (&start, NULL);
    multiply_single(&A, &B, &C1);
    gettimeofday (&end, NULL);
    std::cout << "single end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;

		int p = 1;
do
	{
		std::cout << "pthread with " << p << " threads start..." << std::endl;
    gettimeofday (&start, NULL);
    multiply_pthread(&A, &B, &C2, p);
    gettimeofday (&end, NULL);
    std::cout << "pthread with " << p << " threads end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;

		bool success = validate (&C1, &C2);
		if (C2.val != NULL)
      free(C2.val);
		if (!success)
			break;

    std::cout << "openmp with " << p << " threads start..." << std::endl;
		gettimeofday (&start, NULL);
    multiply_openmp(&A, &B, &C3, p);
    gettimeofday (&end, NULL);
		std::cout << "openmp with " << p << " threads end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;

		success = validate (&C1, &C3);

    if (C3.val != NULL)
      free(C3.val);
		if (!success)
			break;

		p = (p + 2) - (p % 2);
	} while (p <= MAX_THREADS);

    free(A.row);
    free(A.col);
    free(A.val);
    free(B.val);
    if(C1.val != NULL)
        free(C1.val);
    
    return 0;
}

bool validate (struct dense_mtx *C1, struct dense_mtx *C2)
{
	for (uint32_t i = 0; i < C1->nrow; i++)
		{
			for (uint32_t j = 0; j < C1->ncol; j++)
				{
					if (C1->val [C1->ncol * i + j] != C2->val [C2->ncol * i + j])
						{
							std::cout << "Error: Invalid output" << std::endl;
							return false;
						}
				}
		}
	return true;
}
