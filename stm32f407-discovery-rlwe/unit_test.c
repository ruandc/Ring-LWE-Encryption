#include "global.h"
#include "speed_test.h"
#include "term_io.h"
#include "unit_test.h"
#include "lwe.h"
#include "lwe_arm.h"
#include "stdint.h"
#include "stdlib.h"


int compare_uint32(const void * a[2], const void * b[2])
{
	return ( *(uint32_t*)a - *(uint32_t*)b );
}

int compare_uint16(const void * a[2], const void * b[2])
{
   return ( *(uint16_t*)a - *(uint16_t*)b );
}

void convert_to_uint16_t_array(uint32_t *in, uint16_t *out)
{
	int i;
	for (i=0; i<M; i++)
	{
		out[i]=in[i];
	}
}

int memcompare(uint16_t *a, uint16_t *b, int number_of_elements)
{
	int i;
	for (i=0; i<number_of_elements; i++)
	{
		if (a[i]!=b[i])
		{
			xprintf("i=%d\n",i);
			return 0;
		}
	}

	return 1;
}

void perform_unit_tests()
{
	int i,j,fail;
	uint16_t small1[M], small2[M], small3[M];

	// Clear up terminal
	xputs("\n\n[Start]\n\n");

	//rnd = TM_RNG_Get();
	//xprintf("rnd:%u\n",rnd);
/*
	xprintf("knuth_yao_asm_small_tables:");
	fail=0;
	for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
	{
		if ((i%100)==0)
			xprintf(".");
		srand(i);
		knuth_yao_asm_small_tables(large1);

		srand(i);
		knuth_yao2(large2);

		if (!compare_large_simd(large1,large2))
		{
			xprintf("ERRORERRORERROR i=%d \n",i);
			fail=1;
			//break;
		}
	}
	if (fail==1)
	{
		xprintf("FAIL i=%x\n",i);
	}
	else
	{
		xprintf("OK\n");
	}
*/
	#ifdef PERFORM_UNIT_TESTS

	#ifdef PERFORM_UNIT_TESTS_SMALL

	#ifdef USE_FAKE_GET_RAND

		xprintf("knuth_yao_asm_fake_get_rand:");
		fail=0;
		for (i=0xFFFF0000; i<0xffffffff; i++)
		{
			if ((i%100)==0)
				xprintf(".");
			srand(i);
			g_fake_rand=i;
			knuth_yao_asm(large1);

			srand(i);
			knuth_yao2(large2);

			if ((large1[0]>>16)!=large2[1]) //check only the first element
			{
				xprintf("i=%d \n",i);
				fail=1;
				break;
			}
		}
		if (fail==1)
		{
			xprintf("FAIL i=%x\n",i);
		}
		else
		{
			xprintf("OK\n");
		}
	#endif

		xputs("knuth_yao_shuffled: ");
		fail = 0;
		for (i = 0; i < UNIT_TEST_SMALL_LOOPS; i++)
		{
			if ((i%100)==0)
				xprintf(".");

			// Test knuth-yao
			srand(i * i);
			knuth_yao_shuffled(small1);

			srand(i * i);
			knuth_yao_shuffled_with_asm_optimization(small2);

			if (memcompare(small1, small2, M) != 1)
			{
				xprintf("knuth_yao_asm_shuffle fail i=%d\n",i);
				fail = 1;
				break;
			}

			qsort (small1, M, sizeof (uint16_t), compare_uint16);
			qsort (small2, M, sizeof (uint16_t), compare_uint16);
			if (memcompare(small2, small1, M) != 1)
			{
				xputs("knuth_yao_small fail");
				fail = 1;
				break;
			}
		}
		if (fail == 1)
			xputs("BAD!\n");
		else
			xputs("OK!\n");


		xprintf("knuth_yao_asm:");
		fail=0;
		for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
		{
			if ((i%100)==0)
				xprintf(".");
			srand(i);
			knuth_yao_asm(small1);

			srand(i);
			knuth_yao2(small2);

			if (memcompare(small2, small1, M) != 1)
			{
				xprintf("i=%d \n",i);
				fail=1;
				break;
			}
		}
		if (fail==1)
		{
			xprintf("FAIL i=%x\n",i);
		}
		else
		{
			xprintf("OK\n");
		}

		//r2_gen2
		fail = 0;
		xprintf("r2_gen_asm: ");
		for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
		{
			//Test knuth-yao
			srand(i);
			r2_gen_asm(small1);

			srand(i);
			r2_gen2(small2);

			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("BAD: i=%d\n",i);
		else
			xprintf("OK\n");


		xputs("coefficient_add_asm:");
		fail=0;
		for (i=1; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			uint16_t small1_asm[M],small2_asm[M],res_asm[M],res[M];

			srand(i);
			get_small_ntt_random_numbers(small1_asm,small1,i*2);
			get_small_ntt_random_numbers(small2_asm,small2,i*2+1);

			coefficient_add_asm(res_asm,small1,small2);
			coefficient_add2(res,small1,small2);

			if (memcompare(res_asm, res, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xprintf("coefficient_mul_add: ");
		fail=0;
		for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
		{
			uint16_t small1_asm[M],small2_asm[M],small3_asm[M],res_asm[M],res[M];

			srand(i);
			get_small_ntt_random_numbers(small1_asm,small1,i);
			get_small_ntt_random_numbers(small2_asm,small2,i+1);
			get_small_ntt_random_numbers(small3_asm,small3,i+2);

			coefficient_mul_add_asm((uint16_t *)res_asm,(uint16_t *)small1_asm,(uint16_t *)small2_asm,(uint16_t *)small3_asm);

			coefficient_mul_add2(res,small1,small2,small3);

			if (memcompare(res_asm, res, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("BAD!\n");
		else
			xprintf("OK!\n");


		xputs("bitreverse_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			if ((i%100)==0)
				xputc('.');
			get_small_ntt_random_numbers(small1,small2,i);

			bitreverse_asm((uint32_t *)small1);
			bitreverse2(small2);

			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");


		xputs("coefficient_mul_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			uint16_t small1_asm[M],small2_asm[M],res_asm[M],res[M];

			srand(i);
			get_small_ntt_random_numbers(small1_asm,small1,i);
			get_small_ntt_random_numbers(small2_asm,small2,i+1);

			coefficient_mul_asm(res_asm,small1,small2);
			coefficient_mul2(res,small1,small2);

			if (memcompare(res_asm, res, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");


		xputs("a_gen_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			srand(i);
			a_gen_asm(small1);
			srand(i);
			a_gen2(small2);

			//if (!compare_large_simd(large1,large2))
			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xputs("r1_gen_asm:");
		fail=0;
		for (i=7; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			srand(i);
			r1_gen_asm(small1);

			srand(i);
			r1_gen2(small2);

			//if (!compare_large_simd(large1,large2))
			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xputs("coefficient_sub_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			uint16_t small1_asm[M],small2_asm[M],res_asm[M],res[M];

			srand(i);
			get_small_ntt_random_numbers(small1_asm,small1,i);
			get_small_ntt_random_numbers(small2_asm,small2,i+1);

			coefficient_sub_asm(res_asm,small1,small2);
			coefficient_sub2(res,small1,small2);

			if (memcompare(res_asm, res, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xputs("rearrange_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			get_small_ntt_random_numbers(small1,small2,i);
			rearrange_asm(small1);
			rearrange2(small2);

			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xputs("fwd_ntt_parallel_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			get_small_ntt_random_numbers(fixed_data1,small1,i);
			get_small_ntt_random_numbers(fixed_data2,small2,i);
			get_small_ntt_random_numbers(fixed_data3,small3,i);

			fwd_ntt_parallel_asm(fixed_data1);
			fwd_ntt2(small1);
			fwd_ntt2(small2);
			fwd_ntt2(small3);

			if (memcompare(small1, fixed_data1, M) != 1)
			{
				fail=1;
				break;
			}

			if (memcompare(small2, fixed_data2, M) != 1)
			{
				fail=1;
				break;
			}

			if (memcompare(small3, fixed_data3, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xputs("fwd_ntt_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			get_small_ntt_random_numbers(small1,small2,i);

			fwd_ntt_asm(small1);
			fwd_ntt2(small2);

			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xputs("inv_ntt_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			get_small_ntt_random_numbers(small1,small2,i);

			inv_ntt_asm(small1);
			inv_ntt2(small2);

			if (memcompare(small1, small2, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");




		/*
		xprintf("knuth_yao_single_number_big_low_asm:");
		fail=0;
		//for (i=0xF0000000; i<0xFFFFFFFF; i++)
		for (i=0x0; i<0x529b8; i++)
		{
			//srand(i*i);
			//rnd=rand();
			//rnd=906613759; //Goes deep!
			//rnd=1220009855;//Goes toe LUT2
			srand(i*i);
			rnd=i;
			rnd_rem=32;
			//rnd=8191;
			//rnd_rem=13;
			num1=0;
			for (j=0; j<INNER_REPEAT_COUNT; j++)
			{
				num1 += knuth_yao_single_number_big_low_asm(&rnd,&rnd_rem);
			}
			//xprintf("%d ",num);

			srand(i*i);
			//rnd=rand();
			//rnd=906613759; //Goes deep!
			//rnd=1220009855;//Goes toe LUT2
			rnd=i;
			rnd_rem=32;
			//rnd=8191;
			//rnd_rem=13;
			num2=0;
			for (j=0; j<INNER_REPEAT_COUNT; j++)
			{
				num2 += knuth_yao_single_number(&rnd);
			}
			if (num1!=num2)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
		{
			xprintf("FAIL i=%x\n",i);
		}
		else
		{
			xprintf("OK\n");
		}
		*/

		//key_gen_asm
		fail = 0;
		xprintf("key_gen_asm: ");
		for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
		{
			uint16_t small_a[M],small_p[M],small_r2[M],small_a_asm[M],small_p_asm[M],small_r2_asm[M];

			//Test knuth-yao
			srand(i);

			key_gen_asm(small_a,small_p,small_r2);

			srand(i);
			key_gen2(small_a_asm,small_p_asm,small_r2_asm);

			if (memcompare(small_a, small_a_asm, M) != 1)
			{
				fail=1;
				break;
			}

			if (memcompare(small_p, small_p_asm, M) != 1)
			{
				fail=1;
				break;
			}

			if (memcompare(small_r2, small_r2_asm, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");
	#endif //PERFORM_SMALL_TESTS




	#ifdef PERFORM_UNIT_TESTS_BIG
		xprintf("RLWE_enc: ");
		for (i=0; i<UNIT_TEST_BIG_LOOPS; i++)
		{
			uint16_t small_m[M],small_a[M],small_p[M],small_r2[M],small_c1[M],small_c2[M];
			uint16_t small_m_asm[M],small_a_asm[M],small_p_asm[M],small_r2_asm[M],small_c1_asm[M],small_c2_asm[M];

			//Test knuth-yao
			srand(i);
			message_gen2(small_m);
			srand(i);
			key_gen2(small_a,small_p,small_r2); //a=large, p=large2, r2=large3
			srand(i);
			RLWE_enc2(small_a,small_c1,small_c2,small_m,small_p);

			srand(i);
			message_gen_asm(small_m_asm);
			srand(i);
			key_gen_asm(small_a_asm,small_p_asm,small_r2_asm);
			srand(i);
			RLWE_enc_asm(small_a_asm,small_c1_asm,small_c2_asm,small_m_asm,small_p_asm);

			if (memcompare(small_m_asm, small_m, M) != 1)
			{
				xprintf("message");
				fail=1;
				//break;
			}

			if (memcompare(small_a_asm, small_a, M) != 1)
			{
				xprintf("a");
				fail=1;
				//break;
			}

			if (memcompare(small_p_asm, small_p, M) != 1)
			{
				xprintf("p");
				fail=1;
				//break;
			}

			if (memcompare(small_r2_asm, small_r2, M) != 1)
			{
				xprintf("r2");
				fail=1;
				//break;
			}

			if (memcompare(small_c1_asm, small_c1, M) != 1)
			{
				xprintf("c1");
				fail=1;
				//break;
			}

			if (memcompare(small_c2_asm, small_c2, M) != 1)
			{
				xprintf("c2");
				fail=1;
				//break;
			}

			if (fail==1)
				break;
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		xprintf("RLWE_dec: ");
		for (i=0; i<UNIT_TEST_BIG_LOOPS; i++)
		{
			uint16_t small_m[M],small_a[M],small_p[M],small_r2[M],small_c1[M],small_c2[M];
			uint16_t small_m_asm[M],small_a_asm[M],small_p_asm[M],small_r2_asm[M],small_c1_asm[M],small_c2_asm[M];

			//Test knuth-yao
			srand(i);
			message_gen2(small_m);

			srand(i);
			key_gen2(small_a,small_p,small_r2); //a=large, p=large2, r2=large3

			srand(i);
			RLWE_enc2(small_a,small_c1,small_c2,small_m,small_p);

			RLWE_dec2(small_c1,small_c2,small_r2);


			srand(i);
			message_gen_asm(small_m_asm);

			srand(i);
			key_gen_asm(small_a_asm,small_p_asm,small_r2_asm);

			srand(i);
			RLWE_enc_asm(small_a_asm,small_c1_asm,small_c2_asm,small_m_asm,small_p_asm);
			RLWE_dec_asm(small_c1_asm,small_c2_asm,small_r2_asm);

			if (memcompare(small_m_asm, small_m, M) != 1)
			{
				xprintf("message");
				fail=1;
				//break;
			}

			if (memcompare(small_a_asm, small_a, M) != 1)
			{
				xprintf("a");
				fail=1;
				//break;
			}

			if (memcompare(small_p_asm, small_p, M) != 1)
			{
				xprintf("p");
				fail=1;
				//break;
			}

			if (memcompare(small_r2_asm, small_r2, M) != 1)
			{
				xprintf("r2");
				fail=1;
				//break;
			}

			if (memcompare(small_c1_asm, small_c1, M) != 1)
			{
				xprintf("c1");
				fail=1;
				//break;
			}

			if (memcompare(small_c2_asm, small_c2, M) != 1)
			{
				xprintf("c2");
				fail=1;
				//break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		fail = 0;
		xprintf("Enc2/Dec2:");
		for(i=0; (i<UNIT_TEST_BIG_LOOPS) && (fail==0); i++)
		{
			uint16_t small_m[M],small_a[M],small_p[M],small_r2[M],small_c1[M],small_c2[M];
			srand(i);
			message_gen2(small_m);

			bitreverse2(small_m);

			key_gen2(small_a,small_p,small_r2); //a=large, p=large2, r2=large3


			RLWE_enc2(small_a,small_c1,small_c2,small_m,small_p);


			RLWE_dec2(small_c1,small_c2,small_r2);

			for(j=0; j<M; j++)
			{
				if ((small_c1[j]>QBY4) && (small_c1[j]<QBY4_TIMES3))
					small_c1[j] = 1;
				else
					small_c1[j] = 0;
			}

			//Determine if the decryption was correct:
			bitreverse2(small_m);

			rearrange_for_final_test(small_c1,small1);

			if (memcompare(small_m, small1, M) != 1)
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		fail = 0;
		xprintf("Enc_asm/Dec_asm:");
		uint64_t idx = 0;
	#ifdef PERFORM_DECRYPTION_ERROR_TEST
		for(idx=0; (idx<5000000); idx++)
	#else
		for(idx=0; (idx<UNIT_TEST_BIG_LOOPS); idx++)
	#endif
		{
			uint16_t small_m[M],small_a[M],small_p[M],small_r2[M],small_c1[M],small_c2[M];

			if (idx%100==0) xputc('.');

			srand(idx);

			message_gen_asm(small_m);
			key_gen_asm(small_a,small_p,small_r2); //a=large4, p=large5, r2=large6

			bitreverse_asm((uint32_t *)small_m);

			RLWE_enc_asm(small_a,small_c1,small_c2,small_m,small_p);

			RLWE_dec_asm(small_c1,small_c2,small_r2);

			for(j=0; j<M; j++)
			{
				if ((small_c1[j]>QBY4) && (small_c1[j]<QBY4_TIMES3))
					small_c1[j] = 1;
				else
					small_c1[j] = 0;
			}

			//Determine if the decryption was correct:
			bitreverse_asm((uint32_t *)small_m);

			rearrange_for_final_test_asm((uint32_t *)small_c1,(uint32_t *)small1);
			if (memcompare(small_m, small1, M) != 1)
			{
				fail=1;
				break;
			}
		}
	#ifndef PERFORM_DECRYPTION_ERROR_TEST
		if (fail==1)
			xprintf("FAIL idx=%d\n",idx);
		else
			xprintf("OK\n");
	#endif



	#endif //PERFORM_BIG_TESTS


	#endif //PERFORM_TESTS
}
