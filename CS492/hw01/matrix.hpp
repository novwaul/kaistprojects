#ifndef MATRIX_HPP__
#define MATRIX_HPP__

void init_matrix ();

void remove_matrix (double **matrix, int size);
double **make_clean_matrix (int size);
double **make_matrix (int size);

void remove_column (double *col);
double *make_clean_column (int size);
double *make_column (int size);

double **copy_matrix (double **matrix, int size);
double *copy_column (double *col, int size);

#endif /* matrix.hpp ends */
