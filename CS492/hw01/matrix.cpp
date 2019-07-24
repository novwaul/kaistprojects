#include <cfloat>
#include <stdlib.h>
#include <ctime>
#include <string.h>
#include "matrix.hpp"

/* Set a seed for generating random float. */
void init_matrix ()
{
	srand48 (time (NULL));
}

/* Return a n * n matrix initialized to random float. */
double **
make_matrix (int size)
{
  double **matrix = (double **) malloc (size * sizeof (double *));
  for (int k = 0; k < size; k++)
		matrix [k] = (double *) malloc (size * sizeof (double));

  for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
    	matrix [i][j] = drand48 ();

  return matrix;
}

/* Return a n * 1 matrix initialized to random float. */
double *
make_column (int size)
{
	double *col = (double *) malloc (size * sizeof (double));
	
	for (int i = 0; i < size; i++)
		col[i] = drand48 ();

	return col;
}

/* Return a n * n matrix initialized to 0. */
double **
make_clean_matrix (int size)
{
	double **matrix = (double **) malloc (size * sizeof (double *));
	for (int k = 0; k < size; k++)
		matrix [k] = (double *) calloc (size,  sizeof (double));

	return matrix;
}

double *
make_clean_column (int size)
{
	double *col = (double *) calloc (size, sizeof (double));
	return col;
}

/* Remove a matrix. */
void
remove_matrix (double **matrix, int size)
{
	for (int i = 0; i < size; i++)
		free (matrix [i]);
	free (matrix);
}

/* Remove a column. */
void
remove_column (double *col)
{
	free (col);
}

/* Copy a matrix. */
double **
copy_matrix (double **matrix, int size)
{
	double **copy = make_matrix (size);
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			copy [i][j] = matrix [i][j];
	return copy;
}

/* Copy a column. */
double *
copy_column (double *col, int size)
{
	double *copy = make_column (size);
	for (int i = 0; i < size; i++)
		copy [i] = col [i];
	return copy;
}
