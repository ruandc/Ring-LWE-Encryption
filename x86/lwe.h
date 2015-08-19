/*
 * lwe.h
 *
 *  Created on: Jul 3, 2014
 *      Author: rdeclerc
 */

#include <stdint.h>

#ifndef __LWE_H_
#define __LWE_H_

#include "global.h"
#include <stdbool.h>

#ifdef USE_FAKE_GET_RAND
extern uint32_t g_fake_rand;
#endif

bool compare_vectors(uint16_t *a, uint16_t *b);
uint32_t knuth_yao_smaller_tables_single_number(uint32_t *rnd);
uint32_t knuth_yao_single_number_optimized(uint32_t *rnd);
uint32_t knuth_yao_single_number_pmat_optimized();
uint32_t knuth_yao_single_number(uint32_t *rnd, int * sample_in_table);
void knuth_yao_small(uint16_t a[M]);
void knuth_yao_smaller_tables2(uint16_t a[M]);
void bitreverse2(uint16_t a[M]);
void knuth_yao2(uint16_t a[M]);
void knuth_yao_shuffled(uint16_t result[M]);

uint16_t mod(uint32_t a);
uint32_t compare_simd(uint32_t a_0[128], uint32_t a_1[128], uint32_t large[M]);
uint32_t compare_large_simd(uint32_t large_simd[M / 2], uint32_t large[M]);
void coefficient_add2(uint16_t out[M], uint16_t b[M], uint16_t c[M]);
void a_gen2(uint16_t a[]);
void r1_gen2(uint16_t r1[]);
void r2_gen2(uint16_t r2[M]);
void key_gen2(uint16_t a[M], uint16_t p[M], uint16_t r2[M]);
void coefficient_mul_add2(uint16_t *result, uint16_t *large1, uint16_t *large2,	uint16_t *large3);
void coefficient_mul2(uint16_t out[M], uint16_t b[], uint16_t c[]);
void coefficient_sub2(uint16_t result[M], uint16_t b[M], uint16_t c[M]);
void message_gen2(uint16_t m[M]);
void RLWE_enc2(uint16_t a[M], uint16_t c1[M], uint16_t c2[M], uint16_t m[M], uint16_t p[M]);
void RLWE_dec2(uint16_t c1[M], uint16_t c2[M], uint16_t r2[M]);
void fwd_ntt2(uint16_t a[]);
void inv_ntt2(uint16_t a[M]);
void rearrange2(uint16_t a[M]);
void get_ntt_random_numbers(uint32_t *large1, uint32_t *large2, uint32_t i);
void get_small_ntt_random_numbers(uint16_t *small1, uint16_t *small2, uint32_t i);
void rearrange_for_final_test(uint16_t in[M], uint16_t out[M]);
void get_rand_input(uint32_t i, uint32_t large1[M], uint32_t large2[M]);
uint32_t get_rand();
uint32_t get_rand_basic();

#endif /* LWE_H_ */
