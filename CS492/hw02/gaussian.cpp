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

struct maxloc_info
{
	char pad1 [128];
	double val;
	uint32_t loc;
	char pad2 [128];
};

struct gaussian_infomation
{
	struct dense_mtx *A;
	struct dense_mtx *b;
	uint32_t p;
	uint32_t index;
	struct maxloc_info max_info [MAX_THREADS];
	pthread_barrier_t barrier;
	pthread_mutex_t lock;
};

void single (struct dense_mtx *A, struct dense_mtx *b);
void pthread (struct dense_mtx *A, struct dense_mtx *b, uint32_t p);
void openmp (struct dense_mtx *A, struct dense_mtx *b, uint32_t p);
void replicate (struct dense_mtx *A, struct dense_mtx *b, struct dense_mtx *_A, struct dense_mtx *_b);
void L2norm (struct dense_mtx *A, struct dense_mtx *b, struct dense_mtx *_b);
void *gaussian (void *aux);

int 
main (int argc, char** argv)
{
	uint32_t n = atoi (argv[1]);
	uint32_t p = argc == 2 ? 1 : atoi (argv[2]);
	bool on = (argc == 2);
	uint32_t total_size = n * (n + 1);

  struct dense_mtx A, A1, A2, A3, b, b1, b2, b3;

	A.nrow = n;
	A.ncol = n;
	b.nrow = n;
	b.ncol = 1;
	A.val = (double *) malloc (total_size * sizeof (double));
	b.val = (double *) malloc (n * sizeof (double));

	A1.nrow = A2.nrow = A3.nrow = n;
	A1.ncol = A2.ncol = A3.ncol = n;
	b1.nrow = b2.nrow = b3.nrow = n;
	b1.ncol = b2.ncol = b3.ncol = 1;

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

	do
	  {
			A2.val = (double *) malloc (total_size * sizeof (double));
			A3.val = (double *) malloc (total_size * sizeof (double));

			b2.val = (double *) malloc (n * sizeof (double));
			b3.val = (double *) malloc (n * sizeof (double));

			replicate (&A, &b, &A2, &b2);
			replicate (&A, &b, &A3, &b3);

			pthread (&A2, &b2, p);
			L2norm (&A, &b, &b2);

			openmp (&A3, &b3, p);
			L2norm (&A, &b, &b3);

			free (A2.val);
			free (A3.val);

			free (b2.val);
			free (b3.val);

			p = (p + 2) - (p % 2);
		 
	  } while (on && p <= MAX_THREADS);

	free (A.val);
	free (b.val);

	return 0;
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
pthread (struct dense_mtx *A, struct dense_mtx *b, uint32_t p)
{
	struct timeval start, end;
	struct gaussian_infomation ginfo;
	
	pthread_t tids[MAX_THREADS];

	std::cout << "pthread with " << p << " threads start..." << std::endl;
	gettimeofday (&start, NULL);

	pthread_mutex_init (&ginfo.lock, NULL);
	pthread_barrier_init (&ginfo.barrier, NULL, p);

  /* Gaussian elimination. */	
	ginfo.A = A;
	ginfo.b = b;
	ginfo.p = p;
	ginfo.index = 0;
	
	for (uint32_t i = 0; i < p; i++)
		pthread_create (&tids [i], NULL, gaussian, (void *) &ginfo);

	for (uint32_t i = 0; i < p; i++)
		pthread_join (tids [i], NULL);

	/* Back substitution. */
	for (int32_t i = A->nrow - 1; i >= 0; i--)
		{
			b->val [i] /= A->val [A->ncol * i + i];
			for (uint32_t j = i + 1; j < A->ncol; j++)
				b->val [i] -= b->val [j] * A->val [A->ncol * i + j] / A->val [A->ncol * i + i];
		}

	gettimeofday (&end, NULL);
	std::cout << "pthread with " << p << " threads end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
		+ (double) (end.tv_sec - start.tv_sec) << " s." << std::endl;
}

void
openmp (struct dense_mtx *A, struct dense_mtx *b, uint32_t p)
{
	struct timeval start, end;
	struct maxloc_info maxinfo [MAX_THREADS];
	uint32_t mrow, srow, scol;
	double max, temp;

	std::cout << "openmp with " << p << " threads start..." << std::endl;

	gettimeofday (&start, NULL);

	srow = scol = mrow = 0;

	while (srow < (A->nrow - 1))
	  {
			/* Find max location from batch. */
			for (uint32_t i = 0; i < p; i++)
				maxinfo [i].val = 0;
	
			#pragma omp parallel for default (none) shared (maxinfo, A, scol, srow) schedule (guided) num_threads (p)
			for (uint32_t i = srow; i < A->nrow; i++)
				{
					double val = A->val [A->ncol * i + scol];
					double tval, tmax;
					int id = omp_get_thread_num ();

					tval = (val > 0 ? val : -val);
					tmax = (maxinfo [id].val > 0 ? maxinfo [id].val : -maxinfo [id].val);

					if (tval > tmax)
						{
							maxinfo [id].val = val;
							maxinfo [id].loc = i;
						}
				}

			/* Find max location from all.*/
			max = 0;
			for (uint32_t i = 0; i < p; i++)
				{
					double tval, tmax;
					tval = (maxinfo [i].val > 0 ? maxinfo [i].val : -maxinfo [i].val);
					tmax = (max > 0 ? max : -max);
					if (tval > tmax)
						{
							max = maxinfo [i].val;
							mrow = maxinfo [i].loc;
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
			double den = A->val [A->ncol * srow + scol];
				
			#pragma omp parallel default (none) shared (A, b, srow, scol)  num_threads (p)
			{
				uint32_t id = omp_get_thread_num ();
				uint32_t nt = omp_get_num_threads ();
				uint32_t iBlock = (((A->nrow - srow) / nt) / 8 ) * 8;
				uint32_t istart = (id == 0 ? srow + 1 : ((srow + 1 + 7) / 8) * 8 + id * iBlock);
				uint32_t iend = (nt - 1 == id ? A->nrow : ((srow + 1 + 7) / 8) * 8 + (id + 1) * iBlock);

				double den = A->val [A->ncol * srow + scol];
				double m;

				for (uint32_t i = istart; i < iend; i++)
					{
						m = A->val [A->ncol * i + scol] / den;
						for (uint32_t j = scol; j < A->ncol; j++)
							A->val [A->ncol * i + j] -= m * A->val [A->ncol * srow + j];
						b->val [i] -= m * b->val [srow];
					}
			}
				
			srow++;
			scol++;
 	  } /* while end */


	/* Back substitution. */
	for (int32_t i = A->nrow - 1; i >= 0; i--)
		{
			b->val [i] /= A->val [A->ncol * i + i];
			for (uint32_t j = i + 1; j < A->ncol; j++)
				b->val [i] -= b->val [j] * A->val [A->ncol * i + j] / A->val [A->ncol * i + i];
		}


	gettimeofday (&end, NULL);
	std::cout << "openmp with " << p << " threads end: " << (double) (end.tv_usec - start.tv_usec) / 1000000 
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

void *
gaussian (void *aux)
{
	struct gaussian_infomation *ginfo = (struct gaussian_infomation *) aux;
	struct dense_mtx *A, *b;
	uint32_t max, index, p, srow, scol, mrow, istart, iend, iBlock;
	double temp, m, den;

	A = ginfo->A;
	b = ginfo->b;
	p = ginfo->p;

	mrow = srow = scol = 0;
		
	pthread_mutex_lock (&ginfo->lock);
	index = ginfo->index++;
	pthread_mutex_unlock (&ginfo->lock);

	while (scol < A->ncol - 1)
		{
			/* Find max location from batch. */
			max = 0;
			iBlock = (A->nrow - srow) / p;
			istart = (index == 0 ? srow : srow + index * iBlock);
			iend = (index == p - 1 ? A->nrow : srow + (index + 1) * iBlock);

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
		
			ginfo->max_info [index].val = max;
			ginfo->max_info [index].loc = mrow;

			pthread_barrier_wait (&ginfo->barrier);

			if (index == 0)
				{
					/* Find max location from all. */
					max = 0;
					for (uint32_t i = 0; i < p; i++)
						{
							double val = ginfo->max_info [i].val;
							double tval, tmax;

							tval = val > 0 ? val : -val;
							tmax = max > 0 ? max : -max;

							if (tval > tmax)
								{
									max = val;
									mrow = ginfo->max_info [i].loc;
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
				}

			pthread_barrier_wait (&ginfo->barrier);

			iBlock = (((A->nrow - srow) / p) / 8 ) * 8;
			istart = (index == 0 ? srow + 1 : ((srow + 1 + 7) / 8) * 8 + index * iBlock);
			iend = (p - 1 == index ? A->nrow : ((srow + 1 + 7) / 8) * 8 + (index + 1) * iBlock);

			den = A->val [A->ncol * srow + scol];

			/* Gaussian elimination. */	
			for (uint32_t i = istart; i < iend; i++)
				{
					m = A->val [A->ncol * i + scol] / den;
					for (uint32_t j = scol; j < A->ncol; j++)
						A->val [A->ncol * i + j] -= m * A->val [A->ncol * srow + j];
				  b->val [i] -= m * b->val [srow];
				}

			scol++;
			srow++;
		}
}

