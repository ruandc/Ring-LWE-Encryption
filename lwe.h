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

#ifdef USE_FAKE_GET_RAND
extern uint32_t g_fake_rand;
#endif



uint32_t knuth_yao_basic_single_number(uint32_t * rnd);
void knuth_yao_basic_array( uint32_t a[M]);

uint32_t knuth_yao_single_number_lut_optimized(uint32_t * rnd);
void knuth_yao_lut_optimized_array( int a[M]);

void bitreverse2(uint32_t a[M]);


uint32_t mod(int a);
uint32_t compare_simd(uint32_t a_0[128],uint32_t a_1[128],uint32_t large[M]);
uint32_t compare_large_simd(uint32_t large_simd[M/2],uint32_t large[M]);
void coefficient_add2(uint32_t a[M], uint32_t b[M], uint32_t c[M]);
void a_gen2(uint32_t a[]);
void r1_gen2(uint32_t r1[]);
void r2_gen2(uint32_t r2[M]);
void key_gen2(uint32_t a[M], uint32_t p[M], uint32_t r2[M]);
void coefficient_mul_add2(uint32_t * result, uint32_t * large1, uint32_t * large2, uint32_t * large3);
void coefficient_mul2(uint32_t a[M], uint32_t b[], uint32_t c[]);
void coefficient_sub2(uint32_t a[M], uint32_t b[M], uint32_t c[M]);
void message_gen2(uint32_t m[M]);
void RLWE_enc2(uint32_t a[M], uint32_t c1[M], uint32_t c2[M], uint32_t m[M], uint32_t p[M]);
void RLWE_dec2(uint32_t c1[M], uint32_t c2[M], uint32_t r2[M]);
void coefficient_add(uint32_t a_0[], uint32_t a_1[], uint32_t b_0[], uint32_t b_1[]);
void coefficient_mul(uint32_t a_0[], uint32_t a_1[], uint32_t b_0[], uint32_t b_1[]);
void coefficient_sub(uint32_t a_0[], uint32_t a_1[], uint32_t b_0[], uint32_t b_1[]);

void fwd_ntt2(uint32_t a[]);
void inv_ntt2(uint32_t a[M]);

void rearrange2(uint32_t a[M]);

void get_ntt_random_numbers(uint32_t * large1, uint32_t * large2, int i);

void rearrange_decrypted_message(uint32_t in[M],uint32_t out[M]);

void get_rand_input(int i, uint32_t large1[M],uint32_t large2[M]);

#endif /* LWE_H_ */
