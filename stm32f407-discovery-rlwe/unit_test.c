#include <test_asm.h>
#include "global.h"
#include "speed_test.h"
#include "term_io.h"
#include "stdlib.h"
#include "knuth_yao_asm.h"
#include "knuth_yao_asm_shuffle.h"
//#include "knuth_yao_asm_small_tables.h"
#include "unit_test.h"
#include "lwe.h"
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
	int i;
	uint32_t j,num,num1,num2,fail,large1[M],large2[M],large3[M],large4[M],large5[M],large6[M],large_m[M],large_m_asm[M/2],large_c1[M],large_c2[M],large_c1_asm[M/2],large_c2_asm[M/2],large_a[M],large_p[M],large_r2[M];
	uint32_t small1_0[M/2],small1_1[M/2],small2_0[M/2],small2_1[M/2];
	uint32_t rnd,rnd_rem,rnd1,rnd2;
	int num16,num32;

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

			uint16_t small1[M], small2[M], small3[M];

			// Test knuth-yao
			srand(i * i);
			knuth_yao_shuffled(small1);

			srand(i * i);
			knuth_yao_small(small2);

			srand(i * i);
			knuth_yao_asm_shuffle(small3);

			//qsort (small1, M, sizeof (uint16_t), compare_uint16);
			//qsort (small2, M, sizeof (uint16_t), compare_uint16);
			//qsort (small3, M, sizeof (uint16_t), compare_uint16);

			if (memcompare(small3, small1, M) != 1)
			{
				xputs("knuth_yao_asm_shuffle fail");
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


		xputs("knuth_yao_shuffled2: ");
		fail = 0;
		for (i = 0; i < UNIT_TEST_SMALL_LOOPS; i++)
		{
			if ((i%100)==0)
				xprintf(".");
		  // Test knuth-yao
		  srand(i * i);
		  knuth_yao_asm_shuffle(large1);

		  srand(i * i);
		  knuth_yao2(large2);

		  //Sort large1 & large2 and check if they are equivalent
		  //qsort (large1, M, sizeof (uint16_t), (__compar_fn_t)compare_uint16);
		  //qsort (large2, M, sizeof (uint16_t), (__compar_fn_t)compare_uint16);

		  uint16_t small[M];
		  convert_to_uint16_t_array(large2,small);
		  if (memcompare(small, (uint16_t*)large1, M/2) == 0)
		  //if (!compare_large_simd2(large1,large2))
		  {
			  fail = 1;
			  break;
		  }
		}
		if (fail == 1)
			xputs("FAIL!\n");
		else
			xputs("OK!\n");

		xprintf("knuth_yao_asm:");
		fail=0;
		for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
		{
			if ((i%100)==0)
				xprintf(".");
			srand(i);
			knuth_yao_asm(large1);

			srand(i);
			knuth_yao2(large2);

			if (!compare_large_simd(large1,large2))
			{
				xprintf("i=%d \n",i);
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




		xputs("coefficient_add_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			srand(i);
			for (j=0; j<M/2; j++)
			{
				rnd1=rand()&COEFFICIENT_ALL_ONES;
				rnd2=rand()&COEFFICIENT_ALL_ONES;
				rnd1=j+0xFFF;
				rnd2=j+1+0xFFF;
				large1[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
				small1_0[j]=rnd1;
				small1_1[j]=rnd2;

				rnd1=rand();
				rnd2=rand();
				rnd1=j+0xFFF;
				rnd2=j+1+0xFFF;
				large2[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
				small2_0[j]=rnd1;
				small2_1[j]=rnd2;
			}

			coefficient_add_asm(large1,large1,large2);
			coefficient_add(small1_0,small1_1,small2_0,small2_1);

			if (!compare_simd(small1_0,small1_1,large1))
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
			uint32_t large1[M],large2[M],large3[M],large1_asm[M/2],large2_asm[M/2],large3_asm[M/2],res[M],res_asm[M/2];
			//Test knuth-yao
			srand(i);
			int j;
			get_ntt_random_numbers(large1_asm,large1,i);
			get_ntt_random_numbers(large2_asm,large2,i+1);
			get_ntt_random_numbers(large3_asm,large3,i+2);

			coefficient_mul_add_asm(res_asm,large1_asm,large2_asm,large3_asm);

			coefficient_mul_add2(res,large1,large2,large3);

			//if ((!compare(a_0,b_0)) || (!compare(a_1,b_1)))
			if (!compare_large_simd(res_asm,res))
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("BAD!\n");
		else
			xprintf("OK!\n");

		//r2_gen2
		fail = 0;
		xprintf("r2_gen_asm: ");
		for (i=0; i<UNIT_TEST_SMALL_LOOPS; i++)
		{
			//Test knuth-yao
			srand(i);
			r2_gen_asm(large1);

			srand(i);
			r2_gen2(large2);

			if (!compare_large_simd(large1,large2))
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("BAD\n");
		else
			xprintf("OK\n");


		xputs("bitreverse_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			if ((i%100)==0)
				xputc('.');
			get_ntt_random_numbers(large1,large2,i);

			bitreverse_asm(large1);
			bitreverse2(large2);

			if (!compare_large_simd(large1,large2))
			{
				fail=1;
				break;
			}
			xputc('.');
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");


		xputs("coefficient_mul_asm:");
		fail=0;
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			srand(i);
			for (j=0; j<M/2; j++)
			{
				rnd1=rand()&COEFFICIENT_ALL_ONES;
				rnd2=rand()&COEFFICIENT_ALL_ONES;
				rnd1=j+0xFFF;
				rnd2=j+1+0xFFF;
				large1[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
				small1_0[j]=rnd1;
				small1_1[j]=rnd2;

				rnd1=rand();
				rnd2=rand();
				rnd1=j+0xFFF;
				rnd2=j+1+0xFFF;
				large2[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
				small2_0[j]=rnd1;
				small2_1[j]=rnd2;
			}

			coefficient_mul_asm(large1,large1,large2);
			coefficient_mul(small1_0,small1_1,small2_0,small2_1);

			if (!compare_simd(small1_0,small1_1,large1))
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
			a_gen_asm(large1);
			srand(i);
			a_gen2(large2);

			if (!compare_large_simd(large1,large2))
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
		for (i=0; ((i<UNIT_TEST_SMALL_LOOPS) && (fail==0)); i++)
		{
			srand(i);
			r1_gen_asm(large1);

			srand(i);
			r1_gen2(large2);

			if (!compare_large_simd(large1,large2))
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
			srand(i);
			for (j=0; j<M/2; j++)
			{
				rnd1=rand()&COEFFICIENT_ALL_ONES;
				rnd2=rand()&COEFFICIENT_ALL_ONES;
				rnd1=j+0xFFF;
				rnd2=j+1+0xFFF;
				large1[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
				small1_0[j]=rnd1;
				small1_1[j]=rnd2;

				rnd1=rand();
				rnd2=rand();
				rnd1=2*j+0xFFF;
				rnd2=2*j+1+0xFFF;
				large2[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
				small2_0[j]=rnd1;
				small2_1[j]=rnd2;
			}

			coefficient_sub_asm(large1,large1,large2);
			coefficient_sub(small1_0,small1_1,small2_0,small2_1);

			if (!compare_simd(small1_0,small1_1,large1))
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
			get_rand_input(i,large1,large2);

			rearrange_asm(large1);
			rearrange2(large2);

			if (!compare_large_simd(large1,large2))
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
			get_ntt_random_numbers(fixed_data1,large1,i);
			get_ntt_random_numbers(fixed_data2,large2,i);
			get_ntt_random_numbers(fixed_data3,large3,i);

			fwd_ntt_parallel_asm(fixed_data1);
			fwd_ntt2(large1);
			fwd_ntt2(large2);
			fwd_ntt2(large3);

			if (!compare_large_simd(fixed_data1,large1))
			{
				fail=1;
				break;
			}

			if (!compare_large_simd(fixed_data2,large2))
			{
				fail=1;
				break;
			}
			if (!compare_large_simd(fixed_data3,large3))
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
			get_ntt_random_numbers(large1,large2,i);

			fwd_ntt_asm(large1);
			fwd_ntt2(large2);

			if (!compare_large_simd(large1,large2))
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
			get_ntt_random_numbers(large1,large2,i);

			inv_ntt_asm(large1);
			inv_ntt2(large2);

			if (!compare_large_simd(large1,large2))
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
	#endif //PERFORM_SMALL_TESTS




	#ifdef PERFORM_UNIT_TESTS_BIG
		xprintf("RLWE_enc: ");
		for (i=0; i<UNIT_TEST_BIG_LOOPS; i++)
		{
			uint32_t large_a[M],large_p[M],large_r2[M];
			uint32_t large_a_asm[M/2],large_p_asm[M/2],large_r2_asm[M/2];

			//Test knuth-yao
			srand(i);
			message_gen2(large_m);
			srand(i);
			key_gen2(large_a,large_p,large_r2); //a=large, p=large2, r2=large3
			srand(i);
			RLWE_enc2(large_a,large_c1,large_c2,large_m,large_p);

			srand(i);
			message_gen_asm(large_m_asm);
			srand(i);
			key_gen_asm(large_a_asm,large_p_asm,large_r2_asm);
			srand(i);
			RLWE_enc_asm(large_a_asm,large_c1_asm,large_c2_asm,large_m_asm,large_p_asm);

			if (!compare_large_simd(large_m_asm,large_m))
			{
				xprintf("message");
				fail=1;
				//break;
			}

			if (!compare_large_simd(large_a_asm,large_a))
			{
				xprintf("a");
				fail=1;
				//break;
			}
			if (!compare_large_simd(large_p_asm,large_p))
			{
				xprintf("p");
				fail=1;
				//break;
			}

			if (!compare_large_simd(large_c1_asm,large_c1))
			{
				xprintf("c1");
				fail=1;
				//break;
			}

			if (!compare_large_simd(large_c2_asm,large_c2))
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
			//Test knuth-yao
			srand(i);
			message_gen2(large_m);

			srand(i);
			key_gen2(large4,large5,large6); //a=large4, p=large5, r2=large6
			srand(i);
			RLWE_enc2(large4,large_c1,large_c2,large_m,large5);
			RLWE_dec2(large_c1,large_c2,large6);

			srand(i);
			key_gen_asm(large4,large5,large6);//a=large4, p=large5, r2=large6
			srand(i);
			message_gen_asm(large_m_asm);
			srand(i);
			RLWE_enc_asm(large4,large_c1_asm,large_c2_asm,large_m_asm,large5);
			RLWE_dec_asm(large_c1_asm,large_c2_asm,large6);

			if (!compare_large_simd(large_c1_asm,large_c1))
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
		xprintf("Enc2/Dec2:");
		for(i=0; (i<UNIT_TEST_BIG_LOOPS) && (fail==0); i++)
		{
			srand(i);
			message_gen2(large_m);

			bitreverse2(large_m);

			key_gen2(large_a,large_p,large_r2); //a=large, p=large2, r2=large3


			RLWE_enc2(large_a,large_c1,large_c2,large_m,large_p);


			RLWE_dec2(large_c1,large_c2,large_r2);

			for(j=0; j<M; j++)
			{
				if ((large_c1[j]>QBY4) && (large_c1[j]<QBY4_TIMES3))
					large_c1[j] = 1;
				else
					large_c1[j] = 0;
			}

			//Determine if the decryption was correct:
			bitreverse2(large_m);

			rearrange_for_final_test(large_c1,large1);
			//bitreverse2(large_c1);
			for (j=0; j<M; j++)
			{
				if (large_m[j]!=large1[j])
				{
					fail=1;
					break;
				}
			}
			if (fail==1)
				break;
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

		fail = 0;
		xprintf("Enc_asm/Dec_asm:");
		uint32_t error_count = 0;
		uint64_t idx = 0;
	#ifdef PERFORM_DECRYPTION_ERROR_TEST
		for(idx=0; (idx<5000000); idx++)
	#else
		for(idx=0; (idx<UNIT_TEST_BIG_LOOPS); idx++)
	#endif
		{
			if (idx%100==0) xputc('.');

			srand(idx);

			if (idx==0)
			{
				for (j=0; j<M/2; j++)
				{
					if (j<M/4)
					{
						large_m_asm[j]=1+(1<<16);
					}
					else
					{
						large_m_asm[j]=0;
					}
					large4[j]=1+(1<<16);
					large5[j]=1+(1<<16);
					large6[j]=1+(1<<16);
				}
			}
			else
			{
				message_gen_asm(large_m_asm);
				key_gen_asm(large4,large5,large6); //a=large4, p=large5, r2=large6
			}
			bitreverse_asm(large_m_asm);

			RLWE_enc_asm(large4,large_c1_asm,large_c2_asm,large_m_asm,large5);

			RLWE_dec_asm(large_c1_asm,large_c2_asm,large6);
			uint32_t tmp1,tmp2;
			for(j=0; j<M; j++)
			{
				tmp1 = large_c1_asm[j]&0xffff;
				tmp2 = (large_c1_asm[j]>>16)&0xffff;

				if ((tmp1>QBY4) && (tmp1<QBY4_TIMES3))
					tmp1 = 1;
				else
					tmp1 = 0;

				if ((tmp2>QBY4) && (tmp2<QBY4_TIMES3))
					tmp2 = 1;
				else
					tmp2 = 0;

				large_c1_asm[j] = tmp1 + (tmp2<<16);
			}

			//Determine if the decryption was correct:
			bitreverse_asm(large_m_asm);

			rearrange_for_final_test_asm(large_c1_asm,large1);
			for (j=0; j<M/2; j++)
			{
				if (large_m_asm[j]!=large1[j])
				{
					xprintf("idx=%d j=%d ",idx,j);
					fail=1;
					break;
				}
			}
			if (fail==1)
				break;
		}
	#ifndef PERFORM_DECRYPTION_ERROR_TEST
		if (fail==1)
			xprintf("FAIL idx=%d\n",idx);
		else
			xprintf("OK\n");
	#endif

		//key_gen_asm
		fail = 0;
		xprintf("key_gen_asm: ");
		for (i=0; i<UNIT_TEST_BIG_LOOPS; i++)
		{
			//Test knuth-yao
			srand(i);
			key_gen_asm(large1,large2,large3);

			srand(i);
			key_gen2(large4,large5,large6);

			if (!compare_large_simd(large1,large4))
			{
				fail=1;
				break;
			}
			if (!compare_large_simd(large2,large5))
			{
				fail=1;
				break;
			}
			if (!compare_large_simd(large3,large6))
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			xprintf("FAIL i=%x\n",i);
		else
			xprintf("OK\n");

	#endif //PERFORM_BIG_TESTS


	#endif //PERFORM_TESTS
}
