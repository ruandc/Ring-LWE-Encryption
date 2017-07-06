#include <stdint.h>
#include <stdio.h>
#include "lwe.h"
#include "luts.h"
#include "stdlib.h"
#include "assert.h"
#include "limits.h"

uint32_t k_inv;

/*
 * Untested assembly
	.global mod_longa_asm
	.extern mod_longa_asm
	.type mod_longa_asm, %function
mod_longa_asm:
	//a=(in&mask12);
	//r0=in
	ubfx r3,r0,#0,#12; 	  //r3=r0[11:0]
	add r3,r3,r3,lsl #1   //r3=3*r3
	sub r0,r3,r0,lsr #12  //r0=3*r3-r0[31:12}

	mov pc,lr
*/

void fwd_ntt_longa(int32_t a[M]);
void inv_ntt_longa(int32_t a[M]);

void unit_test_longa_poly_mul()
{
	int res = 1;
	int i,j;
	for (i=0; (i<1000) && (res==1); i++)
	{
		uint16_t in1[M],in2[M],out1[M],in3[M],in4[M],out2[M];

		srand(i);
		if (i==0)
		{
			//All ones for first test case
			for (j=0; j<M; j++)
			{
				in1[j]=1;
				in2[j]=1;
			}
		}
		else
		{
			//Random values for other test cases
			for (j=0; j<M; j++)
			{
				in1[j]=rand()%16;
				in2[j]=rand()%16;
			}
		}
		for (j=0; j<M; j++)
		{
			in3[j]=in1[j];
			in4[j]=in2[j];
		}

		fwd_ntt_non_opt(in1);
		fwd_ntt_non_opt(in2);
		coefficient_mul2(out1,in1,in2);
		inv_ntt_non_opt(out1);

		mul_test(in3,in4,out2,M);

		for (j=0; j<M; j++)
		{
			if (out2[j]!=out1[j])
			{
				res=0;

				printf("\n\r out1: ");
				int k;
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, out1[k]);
				printf("\n\r out2: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, out2[k]);
				printf("\n\r");

				break;
			}
		}
	}
	printf("unit_test_longa_poly_mul: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

void unit_test_longa_fwd_inv_ntt()
{
	int res = 1;
	int i,j;
	for (i=0; (i<1000) && (res==1); i++)
	{
		uint32_t in1[M],in2[M];

		srand(i);
		if (i==0)
		{
			//All ones for first test case
			for (j=0; j<M; j++)
			{
				in1[j]=1;
			}
		}
		else
		{
			//Random values for other test cases
			for (j=0; j<M; j++)
			{
				in1[j]=rand()%16;
			}
		}
		for (j=0; j<M; j++)
		{
			in2[j]=in1[j];
		}

		fwd_ntt_longa(in1);
		inv_ntt_longa(in1);

		for (j=0; j<M; j++)
		{
			if (in2[j]!=in1[j])
			{
				res=0;

				break;
			}
		}
	}
	printf("unit_test_longa_fwd_inv_ntt: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

const uint32_t mask12 = ((uint64_t)1 << 12) - 1;


int32_t mod_longa(int32_t in)
{
	uint32_t a;
	int32_t b;
	a=(in&mask12);
	b=(in>>12);
	return 3*a-b;
}

int32_t mod_longa_2x(int32_t in)
{
	uint16_t a,b;
	int16_t c;
	int32_t upper_bound=12289*8*mod(k_inv*k_inv);
	int32_t lower_bound=-12289*8*mod(k_inv*k_inv);

	a=(uint16_t)(in&mask12);
	b=(uint16_t)(in>>12)&mask12;
	c=(int16_t)(in>>24);
	return (9*a-3*b+c);
}

//Modular inverse
int mul_inv(int a, int modulus )
{
	int b0 = modulus, t, q;
	int x0 = 0, x1 = 1;
	if (modulus == 1) return 1;
	while (a > 1) {
		q = a / modulus;
		t = modulus, modulus = a % modulus, a = t;
		t = x0, x0 = x1 - q * x0, x1 = t;
	}
	if (x1 < 0) x1 += b0;
	return x1;
}

/*
void inv_ntt_longa(int32_t a[M], uint16_t k_inv, uint16_t mkinv, uint16_t mod_mkinv, uint16_t k_inv6, uint16_t k_inv7)
{
  int t, h, m, i, j, j1, j2, x=0, ind;
  int *index;
  int16_t S, U, V;

  index = inv_psi1;

  t = 1;
  //for (m = M; m > 1; m = m/2)
  for (m = M; m > 2; m = m/2)
  {
    x++;
    j1 = 0;

    index = index + m/2;
    for (i = 0; i < (m/2); i++)
    {
      j2 = j1 + t -1;
      S = *(index);
      for (j = j1; j<=j2; j++)
      {
        U = a[j];
        V = a[j+t];
        //a[j] = mod_big(U + V);
        //a[j+t] = mod_big((U-V)*S);
        a[j] = U + V;
        a[j+t] = (U-V)*mod(S*k_inv);

        if ((m==128) && (i==28) && (j==226))
        	printf("a");

        if ((m==128) || (m==16))
        {
        	a[j] = mod_longa(a[j]);
        	a[j+t] = mod_longa_2x(a[j+t]);
        }
        else
        {
        	a[j+t] = mod_longa(a[j+t]);
        }
        printf("[%d %d %d] ",m,i,j);
        fflush(stdout);
      }
      j1 = j1 + 2*t;
      index++;
    }
    t = 2*t;
    index = index - m;
  }


  //printf("t=%d\n",t);
  //for (j = 0; j < t; j++)
 // {
  //  //a[j] = mod_big(a[j]*12265);
  //  uint32_t u = a[j];
  //  uint32_t v = a[j+t];
  //  a[j] = mod_longa((u+v)*mkinv);
  //  a[j+t] = mod_longa((u-v)*mod_mkinv);
  //}

  for (j = 0; j < t; j++)
  {
    uint32_t u = a[j];
    uint32_t v = a[j+t];
    a[j] = mod_longa_2x((u+v)*mod(12265*k_inv7));
    a[j+t] = mod_longa_2x((u-v)*mod(12265*inv_psi1[1])*k_inv6);
  }
}
*/

void fwd_ntt_longa(int32_t a[M])
{
  int t, m, i, j, j1, j2, x = 0;
  int32_t S, U, V;

  t = M;
  for(m = 1; m < M; m=2*m)
  {
    x++;
    t = t/2;
    for (i = 0; i < m; i++)
    {
      j1 = 2 * i * t;
      j2 = j1 + t - 1;
      S = psi[m + i];
      for(j = j1; j<=j2; j++)
      {
		U = a[j];
		V = mod(a[j+t] * S);
		a[j] = mod(U + V);
		a[j+t] = mod(U - V);
      }
    }
  }
}

void inv_ntt_longa(int32_t a[M])
{
  int t, h, m, i, j, j1, j2, x=0, ind;
  int *index;
  int32_t S, U, V;

  index = inv_psi1;

  t = 1;
  for (m = M; m > 2; m = m/2)
  {
    x++;
    j1 = 0;

    index = index + m/2;
    for (i = 0; i < (m/2); i++)
    {
      j2 = j1 + t -1;
      S = *(index);
      for (j = j1; j<=j2; j++)
      {
        U = a[j];
        V = a[j+t];
        a[j] = mod_big(U + V);
        a[j+t] = mod_big((U-V)*S);
      }
      j1 = j1 + 2*t;
      index++;
    }
    t = 2*t;
    index = index - m;
  }

  int m_inv=mul_inv(M,12289);
  for (j = 0; j < t; j++)
  {
    uint32_t u = a[j];
    uint32_t v = a[j+t];
    a[j] = mod((u+v)*m_inv);
    a[j+t] = mod((u-v)*mod(m_inv*inv_psi1[1]));
  }
}


void unit_test_mod_longa(int k_inv, int k_inv2)
{
	printf("unit_test_mod_longa: ");
	int i,res;
	for (i=0; i<100000; i++)
	{
		int32_t in;
		if (i==0)
			in = INT_MIN+700000000;
		else if (i==1)
			in = 150994944;
		else
			in=rand();

		int32_t scaled_in=in;

		int32_t output = mod_longa(scaled_in);
		uint32_t output_scaled = mod(output*k_inv); //mod_longa doesn't provide a "true" mod operation. Put it in the same range as expected value

		uint32_t expected = mod(in);

		if (output_scaled!=expected)
		{
			output=mod_longa(output);
			output_scaled = mod(output*k_inv2);

			if (output_scaled!=expected)
			{
				printf("in=%x output=%x expected=%x\n\r",in,output,expected);
				res=0;
				break;
			}
		}
	}
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

void unit_test_mod_longa_2x(int k_inv2)
{
	printf("unit_test_mod_longa_2x: ");
	int i,res;
	res = 1;
	for (i=0; i<1000000; i++)
	{
		int32_t in;
		if (i==0)
			in = INT_MIN;
		else if (i==1)
			in = INT_MAX;
		else
			in=rand();
		int32_t scaled_in=(uint32_t)in;

		int32_t output=mod_longa_2x(scaled_in);
		uint32_t output_scaled = mod(output*k_inv2);

		uint32_t expected = mod(in);

		if (output_scaled!=expected)
		{
			printf("in=%x output=%x output_scaled=%x expected=%x\n\r",in,output,output_scaled,expected);
			res=0;
			break;
		}
	}
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

void unit_test_reduction_longa()
{
	int i;
	int res;

	res = 1;
	k_inv=mul_inv(3, 12289);
	printf("k_inv=%d\n",k_inv);
	uint32_t k_inv2=mod(k_inv*k_inv);

	uint32_t M_inv=mul_inv(512, 12289);
	printf("M_inv=%d\n",M_inv);

	uint32_t tmp=k_inv;
	for (i=0; i<6; i++)
		tmp=mod(tmp*k_inv);
	uint32_t k_inv6=tmp;
	uint32_t k_inv7=mod(tmp*k_inv);

	uint32_t Mk_inv6=mod(M_inv*tmp);

	tmp=mod(tmp*k_inv);
	uint32_t Mk_inv7= M_inv*tmp;
	Mk_inv7=mod(Mk_inv7);

	printf("Mk_inv6=%d\n",Mk_inv7);
	printf("Mk_inv7=%d\n",Mk_inv7);

	uint32_t mod_Mk_inv6= mod(Mk_inv6*inv_psi1[1]);
	printf("mod_Mk_inv6=%d\n",mod_Mk_inv6);	


	unit_test_mod_longa(k_inv, k_inv2);
	unit_test_mod_longa_2x(k_inv2);
	unit_test_longa_fwd_inv_ntt();
}
