#include "global.h"
//#include "luts.h"
#include "lwe.h"
#include "stdio.h"
#include "lwe_original.h"


void main()
{
	int i;
	int j;
	int res;
	uint32_t large_m[M],large_a[M],large_p[M],large_r2[M],large_c1[M],large_c2[M],large1[M],large2[M],a_0[M/2],a_1[M/2];

	res = 1;
	for (i=0; i<0x4FFFF; i++)
	{
		//Test knuth-yao
		srand(i*i);
#ifdef	USE_KNUTH_YAO_ORIGINAL
		knuth_yao_original(a_0,a_1);
#else
		knuth_yao(a_0,a_1);
#endif

		srand(i*i);
		knuth_yao_smaller_tables2(large1);

		if (compare2(a_1,a_0,large1)==0)
		{
			res=0;
			printf("i=%d\n",i);
			break;
		}
	}
	printf("knuth_yao_smaller_tables2: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");


#ifdef USE_FAKE_GET_RAND
	printf("knuth_yao_asm_fake_get_rand:");
	i=0xFFFFF0FF;
	//for (i=0xFFFFF0FF; i<0xffffffff; i++)
	{
		g_fake_rand=i;

		srand(i);
		knuth_yao2(large2);

		printf("i=%d num=%x\n",i,large1[0]>>16);
		return;
	}
#endif

	res = 1;
	for (i=0; i<32767; i++)
	{
		//Test knuth-yao
		srand(i*i);
#ifdef	USE_KNUTH_YAO_ORIGINAL
		knuth_yao_original(a_0,a_1);
#else
		knuth_yao(a_0,a_1);
#endif

		srand(i*i);
		knuth_yao2(large1);

		//if ((!compare(a_0,b_0)) || (!compare(a_1,b_1)))
		if (compare2(a_1,a_0,large1)==0)
			res=0;
	}
	printf("knuth_yao2: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");

	for (i=0; i<327670; i++)
	{
		srand(i*i);
		uint32_t rnd = get_rand();
		uint32_t num1 = knuth_yao_single_number(&rnd);

		srand(i*i);
		rnd = get_rand();
		uint32_t num2 = knuth_yao_smaller_tables_single_number(&rnd);

		if (num1!=num2)
		{
			res=0;
			printf("i=%d\n",i);
			break;
		}
	}
	printf("knuth_yao_smaller_tables_single_number: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");



	res=1;
	res = 1;
	for (i=0; (i<1000) && (res==1); i++)
	{
		//Test knuth-yao
		srand(i);
		if (i==0)
		{
			//a_gen2(large1);
			for (j=0; j<M; j++)
			{
				large1[j]=1;
			}
		}
		else
		{
			for (j=0; j<M; j++)
			{
				large1[j]=rand()%16;
			}
		}

		for (j=0; j<M; j++)
		{
			large2[j]=large1[j];
		}

		fwd_ntt2(large2);
		rearrange2(large2);
		inv_ntt2(large2);
		rearrange2(large2);


		for (j=0; j<M; j++)
		{
			if (large2[j]!=large1[j])
			{
				res=0;
				break;
			}
		}
	}
	printf("fwd/inv_ntt2: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");

	res=1;
	res = 1;
	for (i=0; i<1000; i++)
	{
		//Test knuth-yao
		srand(i);
		a_gen(a_0,a_1);
		//fwd_ntt(a_0,a_1);

		srand(i);
		a_gen2(large1);
		//fwd_ntt2(large1);

		//if ((!compare(a_0,b_0)) || (!compare(a_1,b_1)))
		if (compare2(a_0,a_1,large1)==0)
			res=0;
	}
	printf("fwd_ntt2: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");

	int fail=0;
	printf("Enc/Dec:");
	for(i=0; i<20; i++)
	{
		if ((i%1000)==0)
		{
			printf(".");
			fflush(stdout);
		}
		srand(i);
		if (i==0)
		{
			for (j=0; j<M; j++)
			{
				if (j<M/2)
				{
					large_m[j]=1;
				}
				else
				{
					large_m[j]=0;
				}
				large_a[j]=1;
				large_p[j]=1;
				large_r2[j]=1;
			}
			bitreverse2(large_m);
		}
		else
		{
			message_gen2(large_m);

			bitreverse2(large_m);

			key_gen2(large_a,large_p,large_r2); //a=large, p=large2, r2=large3
		}

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
/*
		for (j=0; j<64; j++)
		{
			if ((large_c1[4*j]!=large_m[2*j]) || (large_c1[4*j+2]!=large_m[2*j+1]))
			{
				printf("Error1!! i=%d,j=%d",i,j);
				fail=1;
				return;
			}
			if ((large_c1[4*j+1]!=large_m[2*(j+64)]) || (large_c1[4*j+3]!=large_m[2*(j+64)+1]))
			{
				printf("Error2!! i=%d,j=%d",i,j);
				fail=1;
				return;
			}
		}
		*/
	}
	if (fail==1)
		printf("BAD!\n");
	else
		printf("OK!\n");

	fail=0;
	for (i=0; i<1000; i++)
	{
		uint32_t large1[M],large2[M],large3[M],large7[M],large8[M];
		//Test knuth-yao
		srand(i);
		int j;
		for (j=0; j<M; j++)
		{
			large1[j]=rand()&0x1fff;
			large2[j]=rand()&0x1fff;
			large3[j]=rand()&0x1fff;
		}
		uint32_t tmp_m[M];
		//coefficient_mul2(tmp_m,a,e1); 		//tmp_m <-- a*e1
		//coefficient_add2(c1, e2, tmp_m);	// c1 <-- e2 <-- e2 + a*e1(tmp_m);
		coefficient_mul2(tmp_m,large1,large2); 		//tmp_m <-- a*e1
		coefficient_add2(large7,tmp_m,large3);

		coefficient_mul_add2(large8,large1,large2,large3);

		//if ((!compare(a_0,b_0)) || (!compare(a_1,b_1)))
		for (j=0; j<M; j++)
			if (large7[j]!=large8[j])
			{
				fail=1;
				break;
			}
	}
	printf("coefficient_mul_add: ");
	if (fail==1)
		printf("BAD!\n");
	else
		printf("OK!\n");
}
