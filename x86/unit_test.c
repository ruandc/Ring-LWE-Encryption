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

#include "lwe.h"
#include "stdlib.h"
#include "global.h"
#include "stdio.h"
#include "unit_test.h"
#include "files.h"

void perform_unit_tests()
{
	int i;
	int j;
	int res;
	uint32_t message[M],a[M],p[M],r2[M],c1[M],c2[M],large1[M],large2[M];

	printf("Test for hex file handling: ");
	res=1;
	for (i=0; i<M; i++)
	{
		c1[i]=rand()&0xffff;
	}
	write_hex_file("hextest.txt",c1,M);
	read_hex_file("hextest.txt",c2,M);
	for (i=0; i<M; i++)
	{
		if (c1[i]!=c2[i])
		{
			printf("i=%d ",i);
			res=0;
			break;
		}
	}
	if (res==0)
		printf("FAIL\n");
	else
		printf("PASS\n");
/*
	uint32_t a[] = {0,1,2,3,4,5,6,7,8,9,10};
	write_hex_file("hextest.txt",a,11);
	read_hex_file("hextest.txt",a,11);

	printf("a=");
	for (i=0; i<11; i++)
		printf("%d ",a[i]);
*/
/*
	printf("knuth_yao: ");
	res = 1;
	for (i=0; i<32767; i++)
	{
		uint32_t rnd,num1,num2;
		//Test knuth-yao
		srand(i*i);
		//knuth_yao_basic_array(large2);
		rnd = rand();
		num1=knuth_yao_basic_single_number(&rnd);

		srand(i*i);
		//knuth_yao_lut_optimized_array(large1);
		rnd=rand();
		num2=knuth_yao_single_number_lut_optimized(&rnd);

		if (num1!=num2)
		{
			res=0;
			break;
		}
	}
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");*/

	printf("Test for Fwd/Inv NTT: ");
	res=1;
	res = 1;
	for (i=0; (i<1000) && (res==1); i++)
	{
		srand(i);

		//Generate a message with a random bit in each element of the array
		for (j=0; j<M; j++)
			large1[j]=rand()%2;

		//Store a copy of large1 inside large2
		for (j=0; j<M; j++)
		{
			large2[j]=large1[j];
		}

		fwd_ntt(large2);
		rearrange(large2);
		inv_ntt(large2);
		rearrange(large2);

		for (j=0; j<M; j++)
		{
			if (large2[j]!=large1[j])
			{
				res=0;
				break;
			}
		}
	}
	if (res==0)
		printf("FAIL\n");
	else
		printf("PASS\n");


	int fail=0;
	printf("Test for Encryption and Decryption: ");
	/*This test procedure tries to determine whether the cryptosystem works.
	 * It does the following tasks:
	 * 1. Generate a random message
	 * 2. Generate a secret and public key pair
	 * 3. Encrypt the message
	 * 4. Decrypt the encrypted message
	 * 5. Check if the decrypted message matches the original
	*/
	for(i=0; i<1000; i++)
	{
		message_gen(message);

		key_gen(a,p,r2);

		rlwe_enc(a,c1,c2,message,p);

		rlwe_dec(c1,c2,r2);

		//Determine if the decryption was correct:
		for (j=0; j<M; j++)
		{
			if (message[j]!=c1[j])
			{
				fail=1;
				break;
			}
		}
		if (fail==1)
			break;
	}
	if (fail==1)
		printf("FAIL\n");
	else
		printf("PASS\n");
}
