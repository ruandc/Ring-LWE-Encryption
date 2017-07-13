#ifndef _REDUCTION_LONGA_3X_H_
#define _REDUCTION_LONGA_3X_H_
#include <stdint.h>


void unit_test_reduction_longa_3x();
int32_t mod_longa(int32_t in);
int32_t mod_longa_2x(int32_t in);
int mul_inv(int a, int modulus );
void fwd_ntt_longa(int32_t a[M]);
void inv_ntt_longa(int32_t a[M], int k_inv_first);
int32_t mul_mod_longa_2x(int32_t in1, int32_t in2);
void correction(int32_t* a, int32_t p, unsigned int N);

#endif
