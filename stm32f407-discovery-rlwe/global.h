#ifndef __GLOBAL_H_
#define __GLOBAL_H_

//#define USE_FAKE_GET_RAND

//#define GENERATE_BINARY_RANDOM_BITS
//#define GENERATE_ASCII_RANDOM_BITS
#define NUMBER_OF_RANDOM_BITS 10000000
#define NUMBER_OF_RANDOM_WORDS NUMBER_OF_RANDOM_BITS/4

#define GENERATE_RANDOM_BITS


//#define DISABLE_KNUTH_YAO
#define INNER_REPEAT_COUNT 1

#define USE_PARALLEL

/*This is a big tests that counts the number of decryption errors that is
 *inherent with the unpadded Ring-LWE encryption scheme */
//#define PERFORM_DECRYPTION_ERROR_TEST

#define PERFORM_UNIT_TESTS
#define PERFORM_UNIT_TESTS_BIG
#define PERFORM_UNIT_TESTS_SMALL

#define PERFORM_SPEED_TESTS
#define PERFORM_BIG_SPEED_TESTS
#define PERFORM_SMALL_SPEED_TESTS

#define UNIT_TEST_BIG_LOOPS 10
#define UNIT_TEST_SMALL_LOOPS 10
#define SPEED_TEST_BIG_LOOPS 1000
#define SPEED_TEST_SMALL_LOOPS 10000

//#define USE_SMALL_TABLES

//#define NTT512
//#define KNUTH_YAO_512

#ifdef KNUTH_YAO_512
	#ifdef USE_SMALL_TABLES
		#define LOW_MSB 31
		#define HAMMING_TABLE_SIZE 21
	#else
		#define LOW_MSB 26
		#define HAMMING_TABLE_SIZE 10 //??? Should this not be 11??
	#endif

	#define PMAT_MAX_COL 106
	#define PMAT_MAX_ROW 58
	#define KN_DISTANCE1_MASK 15
	#define KN_DISTANCE2_MASK 15
#else
	#ifdef USE_SMALL_TABLES
		#define LOW_MSB 31 //Should be named "PMAT_LOW_OCCUPIED_BITS"
		#define HAMMING_TABLE_SIZE 25 //Should be named "PMAT_HIGH_COL_START"
	#else
		#define LOW_MSB 22
		#define HAMMING_TABLE_SIZE 8
	#endif

	#define PMAT_MAX_COL 109
	#define PMAT_MAX_ROW 54
	#define KN_DISTANCE1_MASK 7
	#define KN_DISTANCE2_MASK 15
#endif
#define LOW_MSB_PLUS_ONE LOW_MSB+1
#define PMAT_MAX_ROW_ONE PMAT_MAX_ROW+1

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
	#define SCALING 12265

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
	#define SCALING 7651
	#define COEFFICIENT_ALL_ONES 0x1FFF//13 bits
#endif


//#define USE_TRNG
#define RNG_ADDR 0x50060800

/*
#define NEW_RND_BOTTOM 0
#define NEW_RND_LARGE 32-8
#define NEW_RND_MID 32-5
*/

#define NEW_RND_BOTTOM 1
#define NEW_RND_LARGE 32-9
#define NEW_RND_MID 32-6

#endif
