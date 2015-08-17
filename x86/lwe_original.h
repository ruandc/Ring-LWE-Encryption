/*
 * lwe_original.h
 *
 *  Created on: Sep 1, 2014
 *      Author: rdeclerc
 */

#ifndef LWE_ORIGINAL_H_
#define LWE_ORIGINAL_H_

#include "global.h"
#include "stdint.h"
//#include "luts.h"

void fwd_ntt(int a_0[], int a_1[]);
void a_gen( int a_0[],  int a_1[]);
//int compare2(int a_0[128],int a_1[128],int large[M]);
int compare2(int a_0[128],int a_1[128],uint16_t large[M]);


#endif /* LWE_ORIGINAL_H_ */
