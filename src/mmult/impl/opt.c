/* opt.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes */
#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

/* Alternative Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void* impl_scalar_opt(void* args)
{
	/*Get the argument struct*/
        args_t* passed_args = (args_t*) args;


        /* Get all the arguments */
        float* a = (float*) (passed_args -> inputA);
        float* b = (float*) (passed_args -> inputB);

        float* c = (float*) (passed_args -> output);

        int n = (int)(passed_args -> n);
        int m = (int)(passed_args -> m);
        int p = (int)(passed_args -> p);

	int beta = (int)(passed_args -> beta);

        double temp;

        /* Row Major Implementation of Matrix Multiplication */
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < p; j++) {
			c[i * p + j] = 0;
		}
	}

	for (int ii = 0; ii < n; ii = ii + beta) {
		for (int jj = 0; jj < p; jj = jj + beta) {
			for (int kk = 0; kk < m; kk = kk + beta) {
				int maxi = ii + beta > n? n: ii + beta;
				for (int i = ii; i < maxi; i++) {
					int maxj = jj + beta > p? p: jj + beta;
					for (int j = jj; j < maxj; j++) {
						temp = c[i * p + j];
						int maxk = kk + beta > m? m: kk + beta;
						for (int k = kk; k < maxk; k++) {
							temp += a[i*m + k] * b[k*p + j];
						}
						c[i * p + j] = temp;
					}
				}
			}
		}
	}
        return NULL;
}
#pragma GCC pop_options
