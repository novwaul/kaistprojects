#include "mmreader.hpp"
#include <time.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#define MAX_THREADS 6

struct thread_workinfo
{
	int row;
	pthread_mutex_t *lock;
	struct sparse_mtx *A;
	struct dense_mtx *B;
	struct dense_mtx *C;
};

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
	int32_t row, col, idx, idx_prev, idx_next, offset, i, j;
	uint32_t row_size;
	float value;

	/* Initialize the matrix C. */
	C->nrow = A->nrow;
	C->ncol = B->ncol;
	C->val = (float *) calloc (C->nrow * C->ncol, sizeof (float));

	/* Multiply matrices. */
	row = 0;
	idx = A->row[row];
	idx_prev = -1;
	
	while (row != A->nrow)
		{
			if (idx_prev == idx)
				{
					idx = A->row[++row];
					continue;
				}

			idx_next = A->row[row + 1];
			offset = 0;
			do
				{
					col = A->col[offset + idx];
					value = A->val[offset + idx];
					for (j = 0; j < B->ncol; j++)
						C->val[C->ncol * row + j] += value * B->val[B->ncol * col + j];
				}
			while (idx + (++offset) < idx_next);

			idx_prev = idx;
			idx = A->row[++row];
		}
}

void* update_row (void *ptr)
{
	struct thread_workinfo *info = (struct thread_workinfo *) ptr;
	struct dense_mtx *B, *C;
	struct sparse_mtx *A;
	int row, col, j, idx, idx_next, offset;
	float value;

	pthread_mutex_lock (info->lock);
	row = info->row++;
	pthread_mutex_unlock (info->lock);
	A = info->A;
	B = info->B;
	C = info->C;

	while ((uint32_t) row < A->nrow)
		{
			idx = A->row[row];

			if (row != 0 && idx == A->row[row - 1])
				{
					pthread_mutex_lock (info->lock);
					row = info->row++;
					pthread_mutex_unlock (info->lock);
					continue;
				}
			idx_next = A->row[row + 1];
			offset = 0;
			do
				{
					col = A->col[offset + idx];
					value = A->val[offset + idx];
					for (j = 0; j < B->ncol; j++)
						C->val[C->ncol * row + j] += value * B->val[B->ncol * col + j];
				}
			while (idx + (++offset) < idx_next);

			pthread_mutex_lock (info->lock);
			row = info->row++;
			pthread_mutex_unlock (info->lock);
		}
	return NULL;
}

void multiply_pthread(struct sparse_mtx *A, struct dense_mtx *B, struct dense_mtx *C, int p)
{
	int i;
	struct thread_workinfo info;
	pthread_t tids[5];
	pthread_mutex_t lock;

	/* Initialize the matrix C. */
	C->nrow = A->nrow;
	C->ncol = B->ncol;
	C->val = (float *) calloc (C->nrow * C->ncol, sizeof (float));

	/* Initialize work infomation. */
	pthread_mutex_init (&lock, NULL);
	info.row = 0;
	info.lock = &lock;
	info.A = A;
	info.B = B;
	info.C = C;

	for (i = 0; i < p - 1; i++)
		pthread_create (&tids[i], NULL, update_row, (void *) &info);
	update_row ((void *) &info);
	
	for (i = 0; i < p - 1; i++)
		pthread_join (tids[i], NULL);
}

int main(int argc, char **argv)
{
    struct sparse_mtx A;
		struct timeval begin, end;

    if(!SCsrMatrixfromFile(&A, argv[1]))
    {
        std::cout << "read failed." << std::endl;
        return 0;
    }

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

    struct dense_mtx C1, C2;
    C1.val = NULL;
    C2.val = NULL;

    std::cout << "Single Thread Computation Start" << std::endl;
    gettimeofday (&begin, NULL);
    multiply_single(&A, &B, &C1);
    gettimeofday (&end, NULL);
    std::cout << "Single Thread Computation End: " 
			<< (double) (end.tv_sec - begin.tv_sec) + (double) (end.tv_usec - begin.tv_usec) / 1000000 
			<< " s." << std::endl;

		int p = 1;
		while (true)
			{
    		std::cout << p << " Thread Computation Start" << std::endl;
   	 		gettimeofday (&begin, NULL);
    		multiply_pthread(&A, &B, &C2, p);
    		gettimeofday (&end, NULL);
    		std::cout << "Multi Thread Computation End: " 
					<< (double) (end.tv_sec - begin.tv_sec) + (double) (end.tv_usec - begin.tv_usec) / 1000000 
					<< " s." << std::endl;

				for (int i = 0; i < A.nrow * B.ncol; i++)
					if (C1.val[i] != C2.val[i])
						{
							std::cout << "Error: Results are different." << std::endl;
							 free(A.row);
   						 free(A.col);	
							 free(A.val);
							 free(B.val);
							 if(C1.val != NULL)
       					 free(C1.val);
							 if(C2.val != NULL)
								 free(C2.val);
		  
							 return -1;
						}
				p = (p + 2) - (p % 2);
				if (p > MAX_THREADS)
					break;
				else
					{
						if (C2.val != NULL)
							free (C2.val);
						C2.val = NULL;
					}
			}

    free(A.row);
    free(A.col);
    free(A.val);
    free(B.val);
    if(C1.val != NULL)
        free(C1.val);
    if(C2.val != NULL)
        free(C2.val);
    
    return 0;
}
