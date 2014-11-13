#include "speed_test.h"
#include "stdint.h"
#include <time.h>
#include "test_asm.h"
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "term_io.h"
#include "knuth_yao_asm.h"
#include "stdlib.h"
#include "lwe.h"



void printDuration(uint32_t startTime, uint32_t stopTime, int repeatCount, uint64_t offset_cycles)
{
  uint32_t total_cycles = (stopTime - startTime) - offset_cycles;
  uint32_t average_cycles = total_cycles/repeatCount;
  //xprintf("[Timing] Total: %u %u cycles\n", (unsigned int) (total_cycles >> 32), (unsigned int) total_cycles);
  xprintf("[Timing] Avg: %u cycles\n", average_cycles);
}

void speed_test()
{
	int i;
	uint32_t j,num,num1,num2,fail,large1[M],large2[M],large3[M],large4[M],large5[M],large6[M],large_m[M],large_m_asm[M],large_c1[M],large_c2[M],large_c1_asm[M],large_c2_asm[M];
	uint32_t small1_0[M/2],small1_1[M/2],small2_0[M/2],small2_1[M/2];
	uint32_t rnd,rnd_rem,rnd1,rnd2;
	int num16,num32;

	// Allocate variables
		uint32_t startTime, stopTime;

		//startTime = clock(); // Get the start time
		//stopTime = clock(); // Get the stop time


		CoreDebug->DEMCR = CoreDebug->DEMCR | 0x01000000; //*SCB_DEMCR = *SCB_DEMCR | 0x01000000;
		DWT->CYCCNT=0;//*DWT_CYCCNT = 0; // reset the counter
		DWT->CTRL=DWT->CTRL | 1;//*DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter


		startTime = DWT->CYCCNT; // Get the start time
		stopTime = DWT->CYCCNT; // Get the stop time
		uint64_t offset_cycles = (uint32_t)(stopTime-startTime);
		//uint64_t offset_cycles =0;

		xprintf("offset_cycles:%u\n",(uint32_t)offset_cycles);
		stopTime = DWT->CYCCNT; // Get the stop time

	#ifdef PERFORM_BIG_SPEED_TESTS
		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_BIG_LOOPS; i++)
		{
			ntt_multiply_asm(large1,large2,large3);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("ntt_multiply_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_BIG_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_BIG_LOOPS; i++)
		{
			RLWE_enc_asm(large4,large_c1_asm,large_c2_asm,large_m_asm,large5);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("RLWE_enc_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_BIG_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_BIG_LOOPS; i++)
		{
			RLWE_dec_asm(large_c1_asm,large_c2_asm,large6);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("RLWE_dec_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_BIG_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_BIG_LOOPS; i++)
		{
			key_gen_asm(large1,large2,large3);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("key_gen_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_BIG_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_BIG_LOOPS; i++)
		{
			knuth_yao_asm(large1);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("knuth_yao_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_BIG_LOOPS,offset_cycles);
	#endif

	#ifdef PERFORM_MODULO_SPEED_TESTS
		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num=rand()&0x1fff;
			umod_asm_simd(num);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("mod_asm_simd:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand()&0x1fff;
			umod_asm(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("umod_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand()&0x1fff;
			umod_div_asm(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("umod_div_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand()&0x1fff;
			smod_asm(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("smod_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand();
			if ((num16&1)==1)
				num16=0-(num16&0x3FC01);
			else
				num16=(num16&0x3FC01);
			mod_asm(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("mod_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand();
			if ((num16&1)==1)
				num16=0-(num16&0x3FC01);
			else
				num16=(num16&0x3FC01);
			mod_asm2(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("mod_asm2:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand();
			if ((num16&1)==1)
				num16=0-(num16&0x3FC01);
			else
				num16=(num16&0x3FC01);
			mod_asm3(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("mod_asm3:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand();
			if ((num16&1)==1)
				num16=0-(num16&0x3FC01);
			else
				num16=(num16&0x3FC01);
			mod_asm4(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("mod_asm4:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num16=rand();
			if ((num16&1)==1)
				num16=0-(num16&0x3FC01);
			else
				num16=(num16&0x3FC01);
			mod_asm5(num16);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("mod_asm5:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);

	#endif

	#ifdef PERFORM_SMALL_SPEED_TESTS
		/*
		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		rnd=rand();
		rnd_rem=32;
		for (i=0; i<SPEED_TEST_SMALL_LOOPS; i++)
		{
			num = knuth_yao_single_number_asm(&rnd);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("knuth_yao_single_number_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);
*/
		/*
		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS/100; i++)
		{
			fwd_ntt2(large1);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("fwd_ntt:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS/100,offset_cycles);
*/
		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS/100; i++)
		{
			fwd_ntt_parallel_asm(fixed_data1);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("fwd_ntt_parallel_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS/100,offset_cycles);


		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS/100; i++)
		{
			fwd_ntt_asm(large1);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("fwd_ntt_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS/100,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS/100; i++)
		{
			inv_ntt_asm(large1);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("inv_ntt_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS/100,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS/100; i++)
		{
			coefficient_add_asm(large1,large2,large3);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("coefficient_add_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS/100,offset_cycles);

		srand(1000);
		DWT->CYCCNT=0;
		startTime = DWT->CYCCNT; // Get the start time
		for (i=0; i<SPEED_TEST_SMALL_LOOPS/100; i++)
		{
			coefficient_mul_asm(large1,large2,large3);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		xprintf("coefficient_mul_asm:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS/100,offset_cycles);

		/*
		srand(1000);
		startTime = DWT->CYCCNT; // Get the start time
		rnd=rand();
		for (i=0x00000000; i<=SPEED_TEST_SMALL_LOOPS; i++)
		{
			num = knuth_yao_single_number_optimized(&rnd);
		}
		stopTime = DWT->CYCCNT; // Get the end time
		// Print cycles
		xprintf("knuth_yao_single_number_optimized:");
		printDuration(startTime, stopTime, SPEED_TEST_SMALL_LOOPS,offset_cycles);
		*/
	#endif
}
