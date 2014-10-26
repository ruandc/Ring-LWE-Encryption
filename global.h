/*****************************************************************************
* Copyright 2014 Katholieke Universiteit Leuven
*
* Use of this software is governed by the GNU LGPLv3.0 license
*
* Written by Ruan de Clercq, Sujoy Sinha Roy,
* Frederik Vercauteren, and Ingrid Verbauwhede
*    ______      _____ _____ ______
*   / ____/___  / ___//_  _// ____/
*  / /   / __ \ \__ \  / / / /
* / /___/ /_/ /___/ /_/ /_/ /___
* \____/\____//____//____/\____/
*
* Computer Security and Industrial Cryptography (COSIC)
* K.U.Leuven, Departement Electrical Engineering,
* Celestijnenlaan 200A, B-3001 Leuven, Belgium
****************************************************************************/

#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include "stdint.h"

//#define USE_FAKE_GET_RAND
//#define GENERATE_BINARY_RANDOM_BITS
//#define GENERATE_ASCII_RANDOM_BITS
#define NUMBER_OF_RANDOM_BITS 10000000
#define NUMBER_OF_RANDOM_WORDS NUMBER_OF_RANDOM_BITS/4

#define GENERATE_RANDOM_BITS


//#define DISABLE_KNUTH_YAO
#define INNER_REPEAT_COUNT 1

#define USE_PARALLEL

//#define PERFORM_DECRYPTION_ERROR_TEST

#define PERFORM_UNIT_TESTS
#define PERFORM_UNIT_TESTS_BIG
#define PERFORM_UNIT_TESTS_SMALL


#define PERFORM_SPEED_TESTS
#define PERFORM_BIG_SPEED_TESTS
#define PERFORM_SMALL_SPEED_TESTS


#define UNIT_TEST_BIG_LOOPS 100
#define UNIT_TEST_SMALL_LOOPS 10000
#define SPEED_TEST_BIG_LOOPS 10000
#define SPEED_TEST_SMALL_LOOPS 1000


#define NTT512
//#define KNUTH_YAO_512

#ifdef KNUTH_YAO_512
	#define LOW_MSB 26
	#define HAMMING_TABLE_SIZE 10
	#define PMAT_MAX_COL 106
	#define PMAT_MAX_ROW 58
	#define KN_DISTANCE1_MASK 15
	#define KN_DISTANCE2_MASK 15
#else
	#define LOW_MSB 22
	#define HAMMING_TABLE_SIZE 8
	#define PMAT_MAX_COL 109
	#define PMAT_MAX_ROW 54
	#define KN_DISTANCE1_MASK 7
	#define KN_DISTANCE2_MASK 15
#endif

#ifdef NTT512
	#define MODULUS 12289
	#define M 512
	#define UMOD_CONSTANT 0xAAA71C85
	#define QBY2 6144
	#define QBY4 3072
	#define QBY4_TIMES3 9216

	#define FWD_CONST1 5559
	#define FWD_CONST2 6843

	#define INVCONST1 3778
	#define INVCONST2 10810
	#define INVCONST3 9087
	#define INVERSE_N 12265

	#define COEFFICIENT_ALL_ONES 0x3FFF//14 bits

#else
	#define MODULUS 7681
	#define M 256
	#define UMOD_CONSTANT 0x4441fdcd
	#define QBY2 3840
	#define QBY4 1920
	#define QBY4_TIMES3 5760

	#define FWD_CONST1 5118
	#define FWD_CONST2 1065

	#define INVCONST1 2880
	#define INVCONST2 3383
	#define INVCONST3 2481
	#define INVERSE_N 7651
	#define COEFFICIENT_ALL_ONES 0x1FFF//13 bits
#endif


//#define USE_TRNG
#define RNG_ADDR 0x50060800


#define NEW_RND_BOTTOM 0
#define NEW_RND_LARGE 32-8
#define NEW_RND_MID 32-5

/*
#define NEW_RND_BOTTOM 1
#define NEW_RND_LARGE 32-9
#define NEW_RND_MID 32-6
*/

typedef uint16_t coeff;

#endif
