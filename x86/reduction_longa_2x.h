#ifndef _REDUCTION_LONGA_2X_
#define _REDUCTION_LONGA_2X_

#include <stdint.h>
#include "global.h"

void unit_test_reduction_longa_2x();
void fwd_ntt_longa_2x(int32_t a[M]);
void inv_ntt_longa_2x(int32_t a[M], int k_inv_first);

#endif
