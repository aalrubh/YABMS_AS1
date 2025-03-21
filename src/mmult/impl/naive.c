/* naive.c
 *
 * Author: Ali Al Rubh
 * Date  : 21 Mar. 2025
 *
 *  Description
 *  	This code takes the arguments passed by the main function which are: 3 pointers to matrices (2 input matrices and an output matrix),
 *	and the dimensions of the matrices. Then it computes matrix multiplication based on the dimensions.
 *	Note: This code assumes that the matrices are Row Major
 */

/* Standard C includes */
#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

/* Naive Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void* impl_scalar_naive(void* args)
{
	/*Get the argument struct*/
	args_t passed_args = (args_t *) args;

	/* Get all the arguments */
	float* a = (float*) (passed_args -> inputA);
	float* b = (float*) (passed_args -> inputB);

	float* c = (float*) (passed_args -> output);

	int n = (int)(passed_args -> n);
	int m = (int)(passed_args -> m);
	int p = (int)(passed_args -> p);

	/* Row Major Implementation of Matrix Multiplication */
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			c[i * p + j] = 0;
			for (int k = 0; k < p; k++) {
				c[i * p + j] += a[i*n + k] + b[k * p + j];
			}
		}
	}
	return NULL;
}
#pragma GCC pop_options
