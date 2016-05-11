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
	uint16_t large_m[M],large_a[M],large_p[M],large_r2[M],large_c1[M],large_c2[M],large1[M],large2[M];
	uint32_t a_0[M/2],a_1[M/2];


	printf("knuth_yao_smaller_tables2: ");
	res = 1;
	for (i=0; i<0x4FF; i++)
	{
		if ((i%100)==0)
			printf(".");
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
		srand(i*i);
#ifdef	USE_KNUTH_YAO_ORIGINAL
		knuth_yao_original(a_0,a_1);
#else
		knuth_yao(a_0,a_1);
#endif

		srand(i*i);
		knuth_yao2(large1);

		if (compare2(a_0,a_1,large1)==0)
		{
			res=0;
			break;
		}
	}
	printf("knuth_yao2: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
	int sample_in_table;

	for (i=0; i<327670; i++)
	{
		srand(i*i);
		uint32_t rnd = get_rand();
		uint32_t num1 = knuth_yao_single_number(&rnd,&sample_in_table);

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
		srand(i);
		if (i==0)
		{
			//All ones for first test case
			for (j=0; j<M; j++)
			{
				large1[j]=1;
			}
		}
		else
		{
			//Random values for other test cases
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

	res = 0;
	printf("Enc/Dec:");
	for(i=0; i<10000; i++)
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

		//Determine if the decryption was correct
		bitreverse2(large_m);
		rearrange_for_final_test(large_c1,large1);
		for (j=0; j<M; j++)
		{
			if (large_m[j]!=large1[j])
			{
				res=1;
				break;
			}
		}
		if (res==1)
			break;
	}
	if (res==1)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

