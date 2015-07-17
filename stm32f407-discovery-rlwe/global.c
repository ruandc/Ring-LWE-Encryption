#include "stdint.h"
#include "global.h"
#define attribute_fixed_data1 __attribute__((section(".fixed_data1")));
uint16_t fixed_data1[M] attribute_fixed_data1;
uint16_t fixed_data2[M] attribute_fixed_data1;
uint16_t fixed_data3[M] attribute_fixed_data1;
