/* types.h
 *
 * Author: Ali Al Rubh
 * Date  : 21 Mar. 2025
 *
 * This file contains all required types decalartions.
*/

#ifndef __INCLUDE_TYPES_H_
#define __INCLUDE_TYPES_H_

typedef struct {
	float*  inputA;
	float* 	inputB;
	float*  output;

	int n;
	int m;
	int p;

	size_t size;

	int     cpu;
	int     nthreads;
	int 	beta;
} args_t;

#endif //__INCLUDE_TYPES_H_
