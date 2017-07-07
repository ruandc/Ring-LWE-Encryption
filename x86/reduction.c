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

	//3 cycles


	.global mod_longa_2x_asm
	.extern mod_longa_2x_asm
	.type mod_longa_2x_asm, %function
mod_longa_2x_asm:
	//a=(in&mask12);
	//r0=in
	ubfx r3,r0,#0,#12; 	  //r3=r0[11:0]  (a)
	ubfx r2,r0,#12,#12;   //r2=r0[23:12] (b)


	add r3,r3,r3,lsl #3   //r3=8*r3+r3 = 9*r3 (9*a)
	add r2,r2,r2,lsl #1   //r2=2*r2+r2 = 3*r2 (3*b)

	sub r3,r3,r2  		  //r3=r3-r2 = 9*a-3*b
	add r0,r3,r0,lsr #24  //r0=r3+r0[31:24] = 9*a-3*b+c
	mov pc,lr

	//6 cycles
*/

void fwd_ntt_longa(int16_t a[M]);
void inv_ntt_longa(int16_t a[M], int k_inv_first);


void unit_test_longa_fwd_inv_ntt()
{
	int res = 1;
	int i,j;
	for (i=1; (i<1000) && (res==1); i++)
	{
		int16_t in1[M],in2[M];

		srand(i);
		if (i==0)
			//All ones for first test case
			for (j=0; j<M; j++) in1[j]=1;
		else
			//Random values for other test cases
			for (j=0; j<M; j++) in1[j]=rand()%16;

		for (j=0; j<M; j++)	in2[j]=in1[j];

		fwd_ntt_longa(in1);
		for (j=0; j<M; j++) in1[j]=mod(in1[j]);


		int k_inv_first=k_inv;
		for (j=0; j<7; j++) k_inv_first = mod(k_inv_first*k_inv);

		inv_ntt_longa(in1,k_inv_first);

		for (j=0; j<M; j++)
		{
			if (in2[j]!=mod(in1[j]))
			{
				res=0;
				printf("i=%d\n",i);
				printf("\n\r in1: ");
				int k;
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, mod(in1[k]));
				printf("\n\r in2: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, in2[k]);
				printf("\n\r");

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

void coefficient_mul_longa(uint16_t out[M], uint16_t b[], uint16_t c[]) {
  // a = b * c
  int j;

  for (j = 0; j < M; j++) {
    out[j] = mod_longa((uint32_t)((uint32_t)b[j] * (uint32_t)c[j]));
  }
}

void unit_test_longa_poly_mul()
{
	int res = 1;
	int i,j;
	for (i=0; (i<100) && (res==1); i++)
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

		fwd_ntt_longa(in1);
		for (j=0; j<M; j++) in1[j]=mod(in1[j]);
		fwd_ntt_longa(in2);
		for (j=0; j<M; j++) in1[j]=mod(in1[j]);
		coefficient_mul2(out1,in1,in2);
		//coefficient_mul_longa(out1,in1,in2);

		for (j = 0; j < M; j++) out1[j] = mod(out1[j]);

		int k_inv_first=k_inv;
		for (j=0; j<10; j++) k_inv_first = mod(k_inv_first*k_inv);
		inv_ntt_longa(out1,k_inv_first);

		mul_test(in3,in4,out2,M);

		for (j=0; j<M; j++)
		{
			if (out2[j]!=out1[j])
			{
				res=0;

				printf("\n out1: ");
				int k;
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, mod(out1[k]));
				printf("\n out2: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, out2[k]);
				printf("\n diff:");
				for(k = 0; k< M; k++)
				{
					int aa=mul_inv(out1[k],12289);
					int bb=mod(aa*out2[k]);
					printf("[%d %08d] ", k, bb);
				}

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
	a=(uint16_t)(in&mask12);
	b=(uint16_t)(in>>12)&mask12;
	c=(int16_t)(in>>24);
	return (9*a-3*b+c);
}

int32_t mul_mod_longa_2x(int32_t in1, int16_t in2)
{
	assert(in1<268435456);  //268435456 = 2^28
	assert(in2<16384);		//16384 = 2^14

	uint16_t a,b;
	int32_t c;
	int64_t res1 = (int64_t)in1 * (int64_t)in2;

	a=(uint16_t)(res1&mask12);
	b=(uint16_t)(res1>>12)&mask12;
	c=(int32_t)(res1>>24);
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

void fwd_ntt_longa(int16_t poly[M])
{
  int t, m, i, j, j1, j2, x = 0;
  int32_t S, U, V;
  int32_t a[M];
  int64_t add;
  int64_t mul;
  int32_t mul1;
  uint32_t mul2;


  for (j=0; j<M; j++)
	  a[j]=poly[j];

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
		//U = mod(a[j]);
		//V = mod(a[j+t] * S);

    	if ((m==4) || (m==32) || (m==256))
    	{
			U = mod_longa(a[j]); 					//k
			//V = mod_longa_2x(mul); 					//k
			V = mul_mod_longa_2x(a[j+t],mod(S*k_inv)); 					//k
			a[j] = (U + V);
			a[j+t] = (U - V);

			/*
			if (a[j]!=(int16_t)a[j])
				printf("P");
			if (a[j+t]!=(int16_t)a[j+t])
				printf("P");
				*/
    	}
    	else
    	{
    		U = a[j];								//0
			V = mod_longa(a[j+t] * mod(S*k_inv)); 	//0
			a[j] = (U + V);
			a[j+t] = (U - V);
    	}
      }
    }
  }

  for (j=0; j<M; j++)
  {
	  poly[j]=mod(a[j]);

	  //if (mod(poly[j])!=mod(a[j]))
	  //  printf("AA");
  }
}

void inv_ntt_longa(int16_t poly[M], int k_inv_first)
{
  int t, h, m, i, j, j1, j2, x=0, ind;
  int *index;
  int32_t S, U, V;
  int32_t a[M];

  for (j=0; j<M; j++)
	  a[j]=poly[j];

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
        if ((m==512) || (m==128) || (m==32) || (m==8))
        {
        	a[j] = (U+V);							  //0
        	a[j+t] = mod_longa((U-V) * mod(S*k_inv)); //0
        }
        else
        {
        	a[j] = mod_longa(U+V); 					   //k
        	a[j+t]=mod_longa_2x((U-V) * mod(S*k_inv)); //k
        }
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
    U = a[j];
    V = a[j+t];

    a[j] = mod_longa((U+V) * mod(m_inv*k_inv_first));
    a[j+t] = mod_longa((U-V) * mod(mod(m_inv*inv_psi1[1])*k_inv_first));
  }

  for (j=0; j<M; j++)
	  poly[j]=mod(a[j]); //Need to get rid of this
}



void unit_test_mod_longa(int k_inv, int k_inv2)
{
	printf("unit_test_mod_longa: ");
	int i,res;
	res=1;
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

void unit_test_mul_mod_longa_2x(int k_inv2)
{
	printf("unit_test_mul_mod_longa_2x: ");
	int i,res;
	res = 1;
	for (i=0; i<10000000; i++)
	{
		int32_t in1,in2;
		in1=rand()%268435456;
		in2=rand()%16384;

		int32_t output=mul_mod_longa_2x(in1,in2);
		uint32_t output_scaled = mod(output*k_inv2);

		int64_t tmp = (int64_t)in1*(int64_t)in2;
		uint32_t expected = mod64(tmp);

		if (output_scaled!=expected)
		{
			printf("in1=%x in2=%x output=%x output_scaled=%x expected=%x\n\r",in1,in2,output,output_scaled,expected);
			res=0;
			break;
		}
	}
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

void unit_test_mod_longa_again()
{
	int32_t in1 = 92713;
	int32_t in2 =0x43230bbd;
	int32_t S = 11869;

	int32_t expected_out = mod(92713*11869);

	int32_t output=mod_longa_2x(92713*mod(11869*mod(k_inv*k_inv)));
	output = mod(output);

	int32_t output2=mod_longa(92713*mod(11869*mod(k_inv)));
	int32_t tmp=mod(output2);

	output2=mod_longa(output2*mod(k_inv));
	output2 = mod(output2);
	//tmp2=0x43230bbd
}

void test_overflow()
{
	int32_t u=MODULUS-1;
	int32_t v=-MODULUS+1;
	int64_t uminvw;
	int32_t tmp1;
	uint32_t tmp2;

	int i;
	for (i=0; i<10; i++)
	{
		uminvw=(u-v)*(MODULUS-1);
		tmp1=uminvw;
		tmp2=uminvw;
		if (((int32_t)uminvw)!=uminvw)
		{
			printf("overflow\n");
		}
		int32_t tmp = mod_longa(uminvw);
		int32_t tmp2 = u+v;

		if (abs(tmp2)>abs(tmp))
		{
			tmp = tmp2;
		}
		u = tmp;
		v = -1*tmp;
	}

}

void unit_test_reduction_longa()
{
	int i;
	int res;

	res = 1;
	k_inv=mul_inv(3, 12289);
	printf("k_inv=%d\n",k_inv);
	uint32_t k_inv2=mod(k_inv*k_inv);


	test_overflow();
	unit_test_mod_longa(k_inv, k_inv2);
	unit_test_mod_longa_2x(k_inv2);
	unit_test_mul_mod_longa_2x(k_inv2);
	unit_test_mod_longa_again();
	unit_test_longa_fwd_inv_ntt();
	unit_test_longa_poly_mul();

	printf("DONE");

}
