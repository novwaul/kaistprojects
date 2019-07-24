#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include "matrix.hpp"

#define MAX_THREADS 6

struct info
{
	int n;
	int p;
	int row;
	pthread_mutex_t *lock_row;
	double **A;
	double **B;
	double **C;
};

/* Multiply two matrices. */
void *
multiply_matrix (void *in)
{
	int row_t, j, k, n;
	double **A, **B, **C;
	struct info *info = (struct info *) in;

	n = info->n;
	A = info->A;
	B = info->B;
	C = info->C;

	while (true)
		{
			/* Get # of row to do multiplication. */
			pthread_mutex_lock (info->lock_row);
			row_t = info->row;
			info->row++;
			pthread_mutex_unlock (info->lock_row);

			if (row_t >= n)
				break;

			/* Multiplication. */
			for (j = 0; j < n; j++)
				for (k = 0; k < n; k++)
					C [row_t][j] += A [row_t][k] * B [k][j];
		}
	
	return NULL;	
}

/* Single thread. */
void single_thread (struct info *info)
{
	int i, j, k, n;
	double **A, **B, **C;

	n = info->n;
	A = info->A;
	B = info->B;
	C = info->C;

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			for (k = 0; k < n; k++)
				C [i][j] += A [i][k] * B [k][j];
}

/* Multiple threads. */
void multiple_thread (struct info *info)
{
	int i, p;
	pthread_t tid	[MAX_THREADS - 1];

	p = info->p;

	for (i = 0; i < p - 1; i++)
		 pthread_create (&tid [i], NULL, multiply_matrix, (void *) info);
	multiply_matrix ((void *) info);
	
	for (i = 0; i < p - 1; i++)
		pthread_join (tid [i], NULL);
}

int 
main (int argc, char **argv)
{
	struct timeval begin, end;
	int i, j, n, p;
	double **A, **B, **C1, **C2;
	pthread_mutex_t lock_row;
	struct info info;

	if (argc < 2)
	  {
			std::cout << "Not enough arguments." << std::endl;
			return -1;
	  }
	
	n = atoi (argv[1]);
	info.n = n;
	if (argc > 2)
		{
			if ((p = atoi (argv[2])) > MAX_THREADS)
				{
					std::cout << "Too much threads." << std::endl;
					return -1;
				}
		}
	else
		p = 1;

	info.p = p;
	info.lock_row = &lock_row;
	info.row = 0;

	pthread_mutex_init (&lock_row, NULL);

	/* Matrix generation. */
	init_matrix ();
	A = make_matrix (n);
	B = make_matrix (n);
	C1 = make_clean_matrix (n);
	C2 = make_clean_matrix (n);

	info.A = A;
	info.B = B;
	
	/* Serial version. */
	info.C = C1;

	std::cout << "Single Thread Computation Start" << std::endl;
	gettimeofday (&begin, NULL);
	
	single_thread (&info);

	gettimeofday (&end, NULL);
	std::cout << "Single Thread Computation End: " 
		<< (double) (end.tv_usec - begin.tv_usec) / 1000000  + (double) (end.tv_sec - begin.tv_sec) 
		<< " s." << std::endl;
 

	/* Parallel version. 
	   Do multiplication with 2, 4, and 6 threads. */
	while (true)
		{
			info.C = C2;

			std::cout << info.p << " Threads Computation Start" << std::endl;
			gettimeofday (&begin, NULL);

			multiple_thread (&info);

			gettimeofday (&end, NULL);
			std::cout << "Multi Thread Computation End: " 
				<< (double) (end.tv_usec - begin.tv_usec) / 1000000  + (double) (end.tv_sec - begin.tv_sec) 
				<< " s." << std::endl;
		
			/* Validation. */
			for (i = 0; i < n ;i++)
				for (j = 0; j < n; j++)
					if (C1 [i][j] != C2 [i][j])
						{
							std::cout << "Error: Results are different." << std::endl;
							remove_matrix (A, n);
							remove_matrix (B, n);
							remove_matrix (C1, n);
							remove_matrix (C2, n);
							return -1;
						}

			info.p = (info.p + 2) - (info.p % 2);

			if (info.p > MAX_THREADS || argc > 2)
				break;
			else
				{
					remove_matrix (C2, n);
					C2 = make_clean_matrix (n);
					info.row = 0;
				}
		}

	remove_matrix (A, n);
	remove_matrix (B, n);
	remove_matrix (C1, n);
	remove_matrix (C2, n);

	return 0;
}
