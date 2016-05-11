#include "lwe.h"
#include "luts.h"
#include "stdlib.h"
#include "global.h"
#include <assert.h>
#include <stdbool.h>

#ifdef USE_FAKE_GET_RAND
uint32_t g_fake_rand;
#endif

#ifdef USE_FAKE_GET_RAND
	uint32_t get_rand() { return g_fake_rand; }
#else
	#ifdef USE_TRNG
	#define GET_TRNG_NUMBER (*(volatile unsigned int *) (RNG_ADDR+8));
	uint32_t get_rand()
	{
	  uint32_t rnd = GET_TRNG_NUMBER;
	  rnd |= 0x80000000; // set the least significant bit
	  return rnd;
	}

	uint32_t get_rand_basic()
	{
	  return GET_TRNG_NUMBER;
	}
	#else
		uint32_t get_rand()
		{
		  uint32_t rnd = rand();
		  rnd |= 0x80000000; // set the least significant bit
		  return rnd;
		}

		uint32_t get_rand_basic()
		{
		  return rand();
		}

	#endif
#endif

uint32_t clz(uint32_t a) {
  int i;
  for (i = 0; i < 32; i++) {
    if ((a >> (31 - i)) == 1) {
      return i;
    }
  }
  return 32;
}

/*
 * this function does *not* require -q<x<q
 *
 * returns ret st 0 <= ret < q
 */
uint16_t mod(uint32_t x) {
  int a = (int) x;
  int ret2 = (a % MODULUS) >= 0 ? (a % MODULUS) : (a % MODULUS) + MODULUS;
  while (ret2 < 0) {
    ret2 += MODULUS;
  }
  while (ret2 > MODULUS) {
    ret2 -= MODULUS;
  }
#ifdef DEBUG_PRINTF
  if (!(ret2 >= 0 && ret2 < MODULUS)) {
    printf("error: %d\n", ret2);
  }
#endif
  assert(ret2 >= 0 && ret2 < MODULUS);
  return (uint32_t) ret2;
}

void knuth_yao2(uint16_t a[M]) {
  int i;
  uint32_t rnd;
  int sample_in_table;
  rnd = get_rand();
  for (i = 0; i < M / 2; i++) {
#ifdef DISABLE_KNUTH_YAO
    a[2 * i + 1] = 0;
    a[2 * i] = 0;
#else
    a[2 * i] = knuth_yao_single_number(&rnd,&sample_in_table);
    a[2 * i+1] = knuth_yao_single_number(&rnd,&sample_in_table);
#endif
  }
}

void knuth_yao_shuffled(uint16_t result[M])
{
  int i, sample_in_table;
  uint16_t sample;
  uint32_t rnd;
  int counter1=0;
  int counter2=0;
  rnd = get_rand();
  for (i = 0; i < M / 2; i++) {
#ifdef DISABLE_KNUTH_YAO
	result[2 * i + 1] = 0;
	result[2 * i] = 0;
#else
	sample_in_table=0xff;
    sample = knuth_yao_single_number(&rnd,&sample_in_table);
    //result[2 * i] = sample;

    if (sample_in_table==1) {
    	result[counter1]=sample;
    	 counter1++;
    }
    else
    {
    	result[M-counter2-1]=sample;
    	counter2++;
    }

    sample = knuth_yao_single_number(&rnd,&sample_in_table);
//    result[2 * i+1] = sample;

    if (sample_in_table==1) {
    	result[counter1]=sample;
    	 counter1++;
    }
    else
    {
    	result[M-counter2-1]=sample;
    	counter2++;
    }

#endif
  }


	while (counter2>0) {
		uint32_t rnd = get_rand()&(M-1);//Random number with mask
		if (rnd<(M-counter2))
		{
			//Swap
			sample=result[rnd];
			result[rnd]=result[M-counter2];
			result[M-counter2]=sample;
			counter2--;
		}
	}
}

//#define NEW_RND_BOTTOM 0

#define NEW_RND_BOTTOM 1
#define NEW_RND_LARGE 32 - 9
#define NEW_RND_MID 32 - 6

void knuth_yao_small(uint16_t a[M])
{
  int i;
  uint32_t rnd;
  uint32_t sample_in_table;
  rnd = get_rand();
  for (i = 0; i < M / 2; i++) {
#ifdef DISABLE_KNUTH_YAO
    a[2 * i + 1] = 0;
    a[2 * i] = 0;
#else
    a[2 * i + 1] = knuth_yao_single_number(&rnd,&sample_in_table);
    a[2 * i] = knuth_yao_single_number(&rnd,&sample_in_table);
#endif
  }
}

void knuth_yao_smaller_tables2(uint16_t *a) {
  int i;
  uint32_t rnd;
  rnd = get_rand();
  for (i = 0; i < M / 2; i++) {
#ifdef DISABLE_KNUTH_YAO
    a[2 * i + 1] = 0;
    a[2 * i] = 0;
#else
    a[2 * i + 1] = knuth_yao_smaller_tables_single_number(&rnd);
    a[2 * i] = knuth_yao_smaller_tables_single_number(&rnd);

#endif
  }
}

uint32_t knuth_yao_smaller_tables_single_number(uint32_t *rnd) {
  int distance, row, column, index, sample, sample_msb;

  unsigned int high, low;
#ifdef DEBUG_PRINTF
  printf("Start rnd:%d\n", *rnd);
#endif
  index = (*rnd) & 0xff;
  (*rnd) = (*rnd) >> 8;
  sample = lut1[index]; // M elements in lut1
  sample_msb = sample & 16;
  if (sample_msb == 0) // lookup was successful
  {
    if ((*rnd) == NEW_RND_BOTTOM) {
      (*rnd) = get_rand();
    }
    sample = sample & 0xf;
    if ((*rnd) & 1)
      sample = (MODULUS - sample); // 9th bit in (*rnd) is the sign
    (*rnd) = (*rnd) >> 1;
    // We know that in the next call we will need 8 bits!
    if (clz(*rnd) > (NEW_RND_LARGE)) {
      (*rnd) = get_rand();
    }

    return sample;
  } else {
    if (clz(*rnd) > (NEW_RND_MID)) {
      (*rnd) = get_rand();
    }
    distance = sample & KN_DISTANCE1_MASK;
    index = ((*rnd) & 0x1f) + 32 * distance;
    (*rnd) = (*rnd) >> 5;
    if ((*rnd) == NEW_RND_BOTTOM) {
      (*rnd) = get_rand();
    }
    sample = lut2[index]; // 224 elements in lut2
    sample_msb = sample & 32;
    if (sample_msb == 0) // lookup was successful
    {
      sample = sample & 31;
      if ((*rnd) & 1)
        sample = (MODULUS - sample); // 9th bit in rnd is the sign
      (*rnd) = (*rnd) >> 1;
      if (clz(*rnd) > (NEW_RND_LARGE)) {
        (*rnd) = get_rand();
      }
      return sample;
    } else {
      // Real knuth-yao
      distance = sample & KN_DISTANCE2_MASK;

      // NB: Need to update PMAT_MAX_COL!
      for (column = 0; column < PMAT_MAX_COL; column++) {
        distance = distance * 2 + ((*rnd) & 1);
        (*rnd) = (*rnd) >> 1;
        if ((*rnd) == NEW_RND_BOTTOM) {
          (*rnd) = get_rand();
        }
#ifdef DEBUG_PRINTF
        printf("rnd:%d, dist:%d, col:%d\n", ((*rnd)), distance, column + 13);
#endif
        low = pmat_cols_small_low2[column];

        // if ((int)((unsigned int)distance -
        // pmat_cols_small_hamming[column])<0)
        {
          // Assume that HAMMING_TABLE_SIZE<7 and therefore column<7
          // pmat_cols_small_high only contains a value when column=8 (Real
          // column 20)

          // This means that it must be inside the high part
          // for(row=(54-32); row>=0; row--)
          for (row = (31); row >= 0; row--) {
            distance =
                distance - (low >> 31); // subtract the most significant bit
            low = low << 1;
            if (distance == -1) {
              if ((*rnd) & 1)
                sample = (MODULUS - row);
              else
                sample = row;
              (*rnd) = (*rnd) >> 1;
              if (clz(*rnd) > (NEW_RND_LARGE)) {
                (*rnd) = get_rand();
              }
              return sample;
            }
          }
        }
      }
      for (column = HAMMING_TABLE_SIZE; (column < (109 - 13)); column++) {
        high = pmat_cols_small_high2[column];
        low = pmat_cols_small_low2[column];

        distance = distance * 2 + ((*rnd) & 1);
        (*rnd) = (*rnd) >> 1;
        // if ((column==32)||(column==64)||(column==96))
        if ((*rnd) == NEW_RND_BOTTOM) {
          (*rnd) = rand();
        }
#ifdef DEBUG_PRINTF
        printf("rnd:%d, dist:%d, col:%d\n", ((*rnd)), distance, column);
#endif

        for (row = 54; row >= 32; row--) {
          distance =
              distance - (high >> 31); // subtract the most significant bit
          high = high << 1;

          if (distance == -1) {
            if ((*rnd) & 1)
              sample = (MODULUS - row);
            else
              sample = row;
            (*rnd) = (*rnd) >> 1;
            if (clz(*rnd) > (NEW_RND_LARGE)) {
              (*rnd) = rand();
            }
            return sample;
          }
        }
        // for(row=(54-32); row>=0; row--)
        for (row = (31); row >= 0; row--) {
          distance = distance - (low >> 31); // subtract the most significant
                                             // bit
          low = low << 1;
          if (distance == -1) {
            if ((*rnd) & 1)
              sample = (MODULUS - row);
            else
              sample = row;
            (*rnd) = (*rnd) >> 1;
            if (clz(*rnd) > (NEW_RND_LARGE)) {
              (*rnd) = rand();
            }
            return sample;
          }
        }
      }
    }
  }
  return -1;
}

uint32_t knuth_yao_single_number(uint32_t *rnd, int * sample_in_table)
{
  int distance;
  int row, column;

  uint32_t index, sample, sample_msb;

#ifdef DEBUG_PRINTF
  printf("Start rnd:%x\n", *rnd);
#endif
  index = (*rnd) & 0xff;
  (*rnd) = (*rnd) >> 8;

  sample = lut1[index]; // M elements in lut1
  sample_msb = sample & 16;
  if (sample_msb == 0) // lookup was successful
  {
    if ((*rnd) == NEW_RND_BOTTOM) {
      (*rnd) = get_rand();
    }
    sample = sample & 0xf;
    if ((*rnd) & 1)
      sample = (MODULUS - sample); // 9th bit in rnd is the sign
    (*rnd) = (*rnd) >> 1;
    // We know that in the next call we will need 8 bits!
    if (clz(*rnd) > (NEW_RND_LARGE)) {
      (*rnd) = get_rand();
    }
#ifdef DEBUG_PRINTF
    printf("lut1:%x\n", sample);
#endif
    *sample_in_table=1;
    return sample;
  } else {
    if (clz(*rnd) > (NEW_RND_MID)) {
      (*rnd) = get_rand();
    }
    distance = sample & KN_DISTANCE1_MASK;
    index = ((*rnd) & 0x1f) + 32 * distance;
    (*rnd) = (*rnd) >> 5;
    if ((*rnd) == NEW_RND_BOTTOM) {
      (*rnd) = get_rand();
    }
    sample = lut2[index]; // 224 elements in lut2
    sample_msb = sample & 32;
    if (sample_msb == 0) // lookup was successful
    {
      sample = sample & 31;
      if ((*rnd) & 1)
        sample = (MODULUS - sample); // 9th bit in rnd is the sign
      (*rnd) = (*rnd) >> 1;
      if (clz(*rnd) > (NEW_RND_LARGE)) {
        (*rnd) = get_rand();
      }
#ifdef DEBUG_PRINTF
      printf("lut2:%x\n", sample);
#endif
      *sample_in_table=1;
      return sample;
    } else {
      // Real knuth-yao bitscanning
      distance = sample & KN_DISTANCE2_MASK;
      for (column = 13; (column < PMAT_MAX_COL); column++) {
        distance = distance * 2 + ((*rnd) & 1);
        (*rnd) = (*rnd) >> 1;
        if ((*rnd) == NEW_RND_BOTTOM) {
          (*rnd) = get_rand();
        }
#ifdef DEBUG_PRINTF
        printf("rnd:%x, dist:%d, col:%d\n", (*rnd), distance, column);
#endif

        // Read probability-column 0 and count the number of non-zeros
        for (row = 54; row >= 0; row--) {
          distance = distance - pmat[row][column];
          if (distance < 0) {
#ifdef DEBUG_PRINTF
            printf("rnd:%d", (*rnd));
#endif
            if ((*rnd) & 1)
              sample = (MODULUS - row);
            else
              sample = row;
            (*rnd) = (*rnd) >> 1;
            if (clz(*rnd) > (NEW_RND_LARGE)) {
              (*rnd) = get_rand();
            }

            *sample_in_table=0;
            return sample;
          }
        }
        // rnd = rnd >> 1;
      }
    }
  }

  *sample_in_table=0;
  return 0xffffffff;
}

void a_gen2(uint16_t a[]) {
  uint32_t i, r;

  for (i = 0; i < M / 2; i++) {
    r = (uint32_t)rand();
    a[2 * i] = mod(r & 0xffff);
    a[2 * i + 1] = mod((r >> 16));
  }

  fwd_ntt2(a);
}

void r1_gen2(uint16_t r1[]) {

	#ifdef USE_KNUTH_YAO_SHUFFLE
		knuth_yao_shuffled(r1);
	#else
		knuth_yao2(r1);
	#endif

	fwd_ntt2(r1);
}

void r2_gen2(uint16_t r2[M]) {
  uint16_t i, j, r, bit, sign;

  for (i = 0; i < M;) {
    r = (uint16_t) rand(); // NB: Need to ensure that this is a good source of entropy

    for (j = 0; j < 16; j++) {
      bit = r & 1;
      sign = (r >> 1) & 1;
      if (sign == 1 && bit == 1)
        bit = (MODULUS - 1);
      r2[i++] = bit;
      r = r >> 2;
    }
  }
  fwd_ntt2(r2);
}

void rearrange2(uint16_t a[M]) {
  uint32_t i;
  uint32_t bit1, bit2, bit3, bit4, bit5, bit6, bit7;
  uint32_t swp_index;

  uint16_t u1, u2;

  for (i = 1; i < M / 2; i++) {
    bit1 = i % 2;
    bit2 = (i >> 1) % 2;
    bit3 = (i >> 2) % 2;
    bit4 = (i >> 3) % 2;
    bit5 = (i >> 4) % 2;
    bit6 = (i >> 5) % 2;
    bit7 = (i >> 6) % 2;

#ifdef NTT512
    int bit8 = (i >> 7) % 2;
    swp_index = bit1 * 128 + bit2 * 64 + bit3 * 32 + bit4 * 16 + bit5 * 8 +
                bit6 * 4 + bit7 * 2 + bit8;
#else
    swp_index = bit1 * 64 + bit2 * 32 + bit3 * 16 + bit4 * 8 + bit5 * 4 +
                bit6 * 2 + bit7;
#endif

    if (swp_index > i) {
      u1 = a[2 * i];
      u2 = a[2 * i + 1];

      a[2 * i] = a[2 * swp_index];
      a[2 * i + 1] = a[2 * swp_index + 1];

      a[2 * swp_index] = u1;
      a[2 * swp_index + 1] = u2;
    }
  }
}

bool compare_vectors(uint16_t *a, uint16_t *b)
{
  int i;
  for (i = 0; i < M; i++) {
    if (a[i] != b[i])
      return false;
  }
  return true;
  ;
}

void bitreverse2(uint16_t a[M]) {
  uint32_t i, swp_index;
  uint32_t bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8;
  uint32_t q1, r1, q2, r2;
  uint16_t temp = 0;

  for (i = 0; i < M; i++) {
    bit1 = i % 2;
    bit2 = (i >> 1) % 2;
    bit3 = (i >> 2) % 2;
    bit4 = (i >> 3) % 2;
    bit5 = (i >> 4) % 2;
    bit6 = (i >> 5) % 2;
    bit7 = (i >> 6) % 2;
    bit8 = (i >> 7) % 2;

#ifdef NTT512
    int bit9 = (i >> 8) % 2;
    swp_index = bit1 * 256 + bit2 * 128 + bit3 * 64 + bit4 * 32 + bit5 * 16 +
                bit6 * 8 + bit7 * 4 + bit8 * 2 + bit9;
#else
    swp_index = bit1 * 128 + bit2 * 64 + bit3 * 32 + bit4 * 16 + bit5 * 8 +
                bit6 * 4 + bit7 * 2 + bit8;
#endif
    q1 = i / 2;
    r1 = i % 2;
    q2 = swp_index / 2;
    r2 = swp_index % 2;

    if (swp_index > i) {
      if (r2 == 0)
        temp = a[2 * q2];
      if (r2 == 1)
        temp = a[2 * q2 + 1];
      if (r2 == 0 && r1 == 0)
        a[2 * q2] = a[2 * q1];
      if (r2 == 0 && r1 == 1)
        a[2 * q2] = a[2 * q1 + 1];
      if (r2 == 1 && r1 == 0)
        a[2 * q2 + 1] = a[2 * q1];
      if (r2 == 1 && r1 == 1)
        a[2 * q2 + 1] = a[2 * q1 + 1];
      if (r1 == 0)
        a[2 * q1] = temp;
      if (r1 == 1)
        a[2 * q1 + 1] = temp;
    }
  }
}

void fwd_ntt2(uint16_t a[]) {
  int i, j, k, m;
  uint32_t u1, t1, u2, t2;
  uint32_t primrt, omega;

  i = 0;
  for (m = 2; m <= M / 2; m = 2 * m) {
    primrt = primrt_omega_table[i];
    omega = primrt_omega_table[i + 1];
    i++;

    for (j = 0; j < m; j += 2) {
      for (k = 0; k < M; k = k + 2 * m) {
        u1 = a[j + k];
        t1 = mod(omega * a[j + k + 1]);

        u2 = a[j + k + m];
        t2 = mod(omega * a[j + k + m + 1]);

        a[j + k] = mod(u1 + t1);
        a[j + k + 1] = mod(u2 + t2);

        a[j + k + m] = mod(u1 - t1);
        a[j + k + m + 1] = mod(u2 - t2);

      }
      omega = omega * primrt;
      omega = mod(omega);
    }
  }

  primrt = FWD_CONST1; // mpz_set_str(primrt,"5118",10);
  omega = FWD_CONST2; // mpz_set_str(omega,"1065",10);
  for (j = 0; j < M / 2; j++) {
    t1 = omega * a[2 * j + 1];
    t1 = mod(t1);
    u1 = a[2 * j];
    a[2 * j] = mod(u1 + t1);
    a[2 * j + 1] = mod(u1 - t1);

    omega = omega * primrt;
    omega = mod(omega);
  }
}



void inv_ntt2(uint16_t a[M]) {
  int j, k, m;
  uint32_t u1, t1, u2, t2;
  uint32_t primrt, omega;
  primrt = 0;

  for (m = 2; m <= M / 2; m = 2 * m) {
#ifdef NTT512
    switch (m) {
    case 2:
      primrt = 12288;
      break;
    case 4:
      primrt = 10810;
      break;
    case 8:
      primrt = 7143;
      break;
    case 16:
      primrt = 10984;
      break;
    case 32:
      primrt = 3542;
      break;
    case 64:
      primrt = 4821;
      break;
    case 128:
      primrt = 1170;
      break;
    case 256:
      primrt = 5755;
      break;
    }
#else
    switch (m) {
    case 2:
      primrt = 7680;
      break;
    case 4:
      primrt = 3383;
      break;
    case 8:
      primrt = 5756;
      break;
    case 16:
      primrt = 1728;
      break;
    case 32:
      primrt = 7584;
      break;
    case 64:
      primrt = 6569;
      break;
    case 128:
      primrt = 6601;
      break;
    }
#endif

    omega = 1;
    for (j = 0; j < m / 2; j++) {
      for (k = 0; k < M / 2; k = k + m) {
        t1 = omega * a[2 * (k + j) + 1];
        t1 = mod(t1);
        u1 = a[2 * (k + j)];
        t2 = omega * a[2 * (k + j + m / 2) + 1];
        t2 = mod(t2);
        u2 = a[2 * (k + j + m / 2)];

        a[2 * (k + j)] = mod(u1 + t1);
        a[2 * (k + j + m / 2)] = mod(u1 - t1);

        a[2 * (k + j) + 1] = mod(u2 + t2);
        a[2 * (k + j + m / 2) + 1] = mod(u2 - t2);
      }
      omega = omega * primrt;
      omega = mod(omega);
    }
  }

  primrt = INVCONST1;
  omega = 1;
  for (j = 0; j < M;) {
    u1 = a[j];
    j++;
    t1 = omega * a[j];
    t1 = mod(t1);

    a[j - 1] = mod(u1 + t1);
    a[j] = mod(u1 - t1);
    j++;

    omega = omega * primrt;
    omega = mod(omega);
  }
  uint32_t omega2 = INVCONST2;
  primrt = INVCONST3;
  omega = 1;

  for (j = 0; j < M;) {
    a[j] = mod(omega * a[j]);

    a[j] = mod(a[j] * SCALING);

    j++;
    a[j] = mod(omega2 * a[j]);

    a[j] = mod(a[j] * SCALING);
    j++;

    omega = omega * primrt;
    omega = mod(omega);
    omega2 = omega2 * primrt;
    omega2 = mod(omega2);
  }
}

uint32_t compare_simd(uint32_t a_0[M / 2], uint32_t a_1[M / 2],
                      uint32_t large[M]) {
  int j;
  for (j = 0; j < M / 2; j++) {
    if (((large[j] & 0xffff) != a_0[j]) || ((large[j] >> 16) != a_1[j])) {
      return 0;
    }
  }

  return 1;
}

uint32_t compare_large_simd(uint32_t large_simd[M / 2], uint32_t large[M]) {
  int j;
  for (j = 0; j < M / 2; j++) {
    if (((large_simd[j] & 0xffff) != large[2 * j])) {
      return 0;
    }

    if ((large_simd[j] >> 16) != large[2 * j + 1]) {
      return 0;
    }
  }

  return 1;
}

void coefficient_mul2(uint16_t out[M], uint16_t b[], uint16_t c[]) {
  // a = b * c
  int j;

  for (j = 0; j < M; j++) {
    out[j] = mod((uint32_t)((uint32_t)b[j] * (uint32_t)c[j]));
  }
}

void coefficient_add2(uint16_t out[M], uint16_t b[M], uint16_t c[M])
{
  // a = b + c
  int j;

  for (j = 0; j < M; j++) {
	  out[j] = mod((uint32_t)(b[j] + c[j]));
  }
}

void coefficient_mul_add2(uint16_t *result, uint16_t *large1, uint16_t *large2,	uint16_t *large3) {
  // result=large1*large2+large3
  int j;
  uint32_t tmp;

  for (j = 0; j < M; j++) {
	tmp = large1[j] * large2[j];
    result[j] = mod(tmp + (uint32_t)large3[j]);
  }
}

void coefficient_sub2(uint16_t result[M], uint16_t b[M], uint16_t c[M]) {
  int j;

  for (j = 0; j < M; j++) {
    result[j] = mod(b[j] - c[j]);
  }
}

void key_gen2(uint16_t a[M], uint16_t p[M], uint16_t r2[M]) {
  a_gen2(a);
  r1_gen2(p);
  r2_gen2(r2);

  uint16_t tmp_a[M];

  // a = a*r2
  coefficient_mul2(tmp_a, a, r2);
  // p = p-a*r2
  coefficient_sub2(p, p, tmp_a);

  rearrange2(r2);
}

void RLWE_enc2(uint16_t a[M], uint16_t c1[M], uint16_t c2[M], uint16_t m[M], uint16_t p[M])
{
  int i;
  uint16_t e1[M], e2[M], e3[M];
  uint16_t encoded_m[M];
  for (i = 0; i < M; i++) {
    encoded_m[i] = m[i] * QBY2; // encoding of message
  }

#ifdef USE_KNUTH_YAO_SHUFFLE
	knuth_yao_shuffled(e1);
	knuth_yao_shuffled(e2);
	knuth_yao_shuffled(e3);
#else
	knuth_yao2(e1);
	knuth_yao2(e2);
	knuth_yao2(e3);
#endif

  coefficient_add2(e3, e3, encoded_m); // e3 <-- e3 + m

  fwd_ntt2(e1);
  fwd_ntt2(e2);
  fwd_ntt2(e3);

  // m <-- a*e1
  coefficient_mul2(c1, a, e1);  // c1 <-- a*e1
  coefficient_add2(c1, e2, c1); // c1 <-- e2 + a*e1(tmp_m);
  coefficient_mul2(c2, p, e1);  // c2 <-- p*e1
  coefficient_add2(c2, e3, c2); // c2<-- e3 + p*e1

  rearrange2(c1);
  rearrange2(c2);
}

void RLWE_dec2(uint16_t c1[M], uint16_t c2[M], uint16_t r2[M])
{
  coefficient_mul2(c1, c1, r2); // c1 <-- c1*r2
  coefficient_add2(c1, c1, c2); // c1 <-- c1*r2 + c2

  inv_ntt2(c1);
}

void message_gen2(uint16_t m[M]) {
  int i;
  for (i = 0; i < M; i++) {
    m[i] = (uint16_t)(get_rand() % 2);
  }
}

void get_small_ntt_random_numbers(uint16_t *small1, uint16_t *small2, uint32_t i)
{
	uint32_t j;
	uint32_t rnd1, rnd2;

	srand(i);
	for (j = 0; j < M / 2; j++) {
	  rnd1 = get_rand() & 0x1FFF;
	  rnd2 = get_rand() & 0x1FFF;
	  small1[2*j] = mod(rnd1);
	  small1[2*j+1] = mod(rnd2);
	  small2[2 * j] = mod(rnd1);
	  small2[2 * j + 1] = mod(rnd2);
	}
}

void get_ntt_random_numbers(uint32_t *large1, uint32_t *large2, uint32_t i) {
  uint32_t j;
  uint32_t rnd1, rnd2;

  srand(i);
  if (i == 0) {
    for (j = 0; j < M / 2; j++) {
      rnd1 = j + 1;
      rnd2 = j + 2;
      large1[j] = (rnd1 & 0xffff) + ((rnd2 & 0xffff) << 16);
      large2[2 * j] = rnd1;
      large2[2 * j + 1] = rnd2;
    }
  } else {
    for (j = 0; j < M / 2; j++) {
      rnd1 = get_rand() & 0x1FFF;
      rnd2 = get_rand() & 0x1FFF;
      large1[j] = (rnd1 & 0xffff) + ((rnd2 & 0xffff) << 16);
      large2[2 * j] = rnd1;
      large2[2 * j + 1] = rnd2;
    }
  }
}

void rearrange_for_final_test(uint16_t in[M], uint16_t out[M])
{
  int i;
  for (i = 0; i < M / 2; i += 2) {
    out[i] = in[2 * i];
    out[i + 1] = in[2 * (i + 1)];
  }

  for (i = 0; i < M / 2; i += 2) {
    out[i + M / 2] = in[2 * i + 1];
    out[i + 1 + M / 2] = in[2 * (i + 1) + 1];
  }
}

void get_rand_input(uint32_t i, uint32_t large1[M], uint32_t large2[M]) {
  uint32_t rnd1, rnd2, j;
  srand(i);
  if (i == 0) {
    for (j = 0; j < M / 2; j++) {
      rnd1 = 2 * j;
      rnd2 = 2 * j + 1;
      large1[j] = (rnd1 & 0xffff) + ((rnd2 & 0xffff) << 16);
      large2[2 * j] = rnd1;
      large2[2 * j + 1] = rnd2;
    }
  } else {
    for (j = 0; j < M / 2; j++) {
      rnd1 = get_rand() & COEFFICIENT_ALL_ONES;
      rnd2 = get_rand() & COEFFICIENT_ALL_ONES;
      large1[j] = (rnd1 & 0xffff) + ((rnd2 & 0xffff) << 16);
      large2[2 * j] = rnd1;
      large2[2 * j + 1] = rnd2;
    }
  }
}
