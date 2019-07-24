#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <pthread.h>
#include <sys/time.h>
#include "matrix.hpp"

#define MAX_THREADS 6

struct info
{
	int n;
	int p;
	int fixpoint;
	int task;
	double **A;
	double *b;
	double *x;
	pthread_mutex_t *lock_task;
	pthread_mutex_t *lock_x;
};

/* Do row opertaion for a row. */
void *
row_operation (void *aux)
{
	int row, col, row_t, j, n;
	double m, *b, **A;
	struct info *info = (struct info *) aux;

	row = col = info->fixpoint;
	n = info->n;
	A = info->A;
	b = info->b;

	/* Get start # of row and update TASK. */
	pthread_mutex_lock (info->lock_task);
	row_t = n - info->task; 
	info->task--;
	pthread_mutex_unlock (info->lock_task);

	while (row_t < n)
		{
			m = A [row_t][col] / A [row][col];
			for (j = col; j < n; j++)
				A [row_t][j] -= m * A [row][j];
			b [row_t] -= m * b [row];
		
			pthread_mutex_lock (info->lock_task);
			row_t = n - info->task; 
			info->task--;
			pthread_mutex_unlock (info->lock_task);
		}
	
	return NULL;
}

/* Do back substitution for a row. */
void *
back_sub (void *aux)
{
	int col_t, col, row, n, p;
	double result;
	double *x, *b, **A;
	struct info *info = (struct info *) aux;

	row = col = info->fixpoint;
	n = info->n;
	p = info->p;
	A = info->A;
	b = info->b;
	x = info->x;

	/* Get start # of column and update TASK. */
	pthread_mutex_lock (info->lock_task);
	col_t = (col + 1) + (p - info->task);
	info->task--;
	pthread_mutex_unlock (info->lock_task);

	result = 0;
	while (col_t < n)
		{
			result -= A [row][col_t] * x [col_t] / A [row][col];
			col_t += p;
		}	

	if (col_t == n)
		result += b [row] / A [row][col];

	pthread_mutex_lock (info->lock_x);
	x[row] += result;
	pthread_mutex_unlock (info->lock_x);

	return NULL;
}

/* Return the position where maximum value exists.*/
int
maxloc (int size, int col, double **A)
{
	int i, pos, row;
	double abs, max = 0;

	pos = row = col;
	for (i = row; i < size; i++)
		{
			if (A [i][col] >= 0)
				abs = A[i][col];
			else
				abs = -A [i][col];

			if (abs > max)
				{
					max = abs;
					pos = i;
				}
		}
	
	return pos;
}

/* Swap row vectors. */
void
swap_matrix (int row1, int row2, double **A)
{
	double *temp;
	
	if (row1 == row2)
		return;

	temp = A [row1];
	A [row1] = A [row2];
	A [row2] = temp;

	return;
}

/* Swap row scalars. */
void 
swap_col (int row1, int row2, double *b)
{
	double temp;

	if (row1 == row2)
		return;

	temp = b [row1];
	b [row1] = b [row2];
	b [row2] = temp;

	return;
}

/* Single thread version. */
void single_thread (struct info *info)
{
	int col, row, pos, i, j, n;
	double **A, *b, *x, m;

	n = info->n;
	A = info->A;
	b = info->b;
	x = info->x;

	/* Gaussian elimination. */
	for (col = 0; col < n; col++)
		{
			pos = maxloc (n, col, A);
			row = col;
			swap_matrix (row, pos, A);
			swap_col (row, pos, b);
			for (i = row + 1; i < n; i++)
				{
					m = A [i][col] / A [row][col];
					for (j = col; j < n; j++)
						A [i][j] -= m * A [row][j];
					b [i] -= m * b [row];
				}
		}

	/* Back substitution. */
	for (i = 0; i < n; i++)
		{
			for (j = 0; j < i; j++)
				x [n - 1 - i] -= A [n - 1 - i][n - 1 - j] * x [n - 1 - j];
			x [n - 1 - i] += b [n - 1 - i];
			x [n - 1 - i] /= A [n - 1 - i][n - 1 - i];
		}

}

/* Multiple threads version. */
void multiple_thread (struct info *info)
{
	int col, row, pos, n, p, id;
	pthread_t tid	[MAX_THREADS - 1];

	n = info->n;
	p = info->p;

	for (col = 0; col < n; col++)
		{
			pos = maxloc (n, col, info->A);
			row = col;
			swap_matrix (row, pos, info->A);
			swap_col (row, pos, info->b);

			/* Do row operation. */
			info->task = (n - 1) - col;
			info->fixpoint = col;
			for (id = 0; id < p - 1; id++)
				pthread_create (&tid [id], NULL, row_operation, (void *) info);
			row_operation ((void *) info);
			
			for (id = 0; id < p - 1; id++)
				pthread_join (tid [id], NULL);
		}

	for (row = n - 1; row >= 0; row--)
		{
			/* Do back substitution. */
			info->task = p;
			info->fixpoint = row;
			for (id = 0; id < p - 1; id++)
				pthread_create (&tid [id], NULL, back_sub, (void *) info);
			back_sub ((void *) info);
			for (id = 0; id < p - 1; id++)
				pthread_join (tid [id], NULL);
		}
}

/* Validate computation. */
void validate (double **A, double *b, double *x, int n)
{
	double residual, quasi_b;
	int i, j;

	/* Calulate L2-norm. */
	residual = 0;
	for (i = 0; i < n; i++)
		{
			quasi_b = 0;
			for (j = 0; j < n; j++)
				quasi_b += A[i][j] * x[j];
			residual += (quasi_b - b[i]) * (quasi_b - b[i]);
		}
	std::cout << "L2-norm: " << sqrt (residual) << std::endl;
}

int
main (int argc, char **argv)
{
	double *b, **A;
	struct timeval begin, end;
	struct info info;
	int n, p;							
	pthread_mutex_t lock_task; 	
	pthread_mutex_t lock_x;			

	if (argc < 2)
		{
			std::cout << "Not enough arguments." << std::endl;	
			return -1;
		}

	n = atoi (argv[1]);
	if (argc == 2)
		p = 1;
	else
		{
			if ((p = atoi (argv[2])) > MAX_THREADS)
				{
					std::cout << "Too many threads." << std::endl;
					return -1;
				}
		}

	info.n = n;
	info.p = p;

	init_matrix ();
	A = make_matrix (n);
	b = make_column (n);
	
	info.x = make_clean_column (n);
	info.A = copy_matrix (A, n);
	info.b = copy_column (b, n);
	
	std::cout << "Single Thread Computation Start" << std::endl;
	gettimeofday (&begin, NULL);
	
	single_thread (&info);

	gettimeofday (&end, NULL);
	std::cout << "Single Thread Computation End: " 
		<< (double) (end.tv_usec - begin.tv_usec) / 1000000  + (double) (end.tv_sec - begin.tv_sec) 
		<< " s." << std::endl;

	validate (A, b, info.x, n);
	
	remove_matrix (info.A, n);
	remove_column (info.b);
	remove_column (info.x);
	
	info.x = make_clean_column (n);
	info.A = copy_matrix (A, n);
	info.b = copy_column (b, n);

	pthread_mutex_init (&lock_task, NULL);
	pthread_mutex_init (&lock_x, NULL);

	info.lock_task = &lock_task;
	info.lock_x = &lock_x;

	while (true)
		{
			std::cout << info.p << " Thread Computation Start" << std::endl;
			gettimeofday (&begin, NULL);
	
			multiple_thread (&info);

			gettimeofday (&end, NULL);
			std::cout << "Multi Thread Computation End: " 
				<< (double) (end.tv_usec - begin.tv_usec) / 1000000  + (double) (end.tv_sec - begin.tv_sec) 
				<< " s." << std::endl;

			validate (A, b, info.x, n);

			info.p = (info.p + 2) - (info.p % 2);
			if (info.p > MAX_THREADS || argc > 2)
				break;
			else
				{
					remove_matrix (info.A, n);
					remove_column (info.b);
					remove_column (info.x);

					info.x = make_clean_column (n);
					info.A = copy_matrix (A, n);
					info.b = copy_column (b, n);
				}
		}

	remove_matrix (info.A, n);
	remove_column (info.b);
	remove_column (info.x);

	remove_matrix (A, n);
	remove_column (b);

	return 0;
}
