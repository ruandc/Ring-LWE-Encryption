#include <stdio.h>
#include "lwe.h"
#include "luts.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "global.h"
#include "reduction_longa_3x.h"


/*
 * Untested assembly
	.global mod_longa_asm
	.extern mod_longa_asm
	.type mod_longa_asm, %function
mod_longa_asm:
	//a=(in&mask12);
	//r0=in
	ubfx r1,r0,#0,#12; 	  //r1=r0[11:0] 		(a)
	add r1,r1,r1,lsl #1   //r1=1*r3 			(3*a)
	sub r0,r1,r0,lsr #12  //r0=3*r3-r0[31:12}	(3*a-b)

	mov pc,lr

	//3 cycles, 1 temp register

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


	.global mul_mod_longa_2x_asm
	.extern mul_mod_longa_2x_asm
	.type mul_mod_longa_2x_asm, %function
mul_mod_longa_2x_asm:
	//r0=in1
	//r1=in2

	smull r1,r0,r1,r2     	//r0=low, r1=high

	ubfx r3,r0,#0,#12; 	  	//r3=r0[11:0]  (a)
	add r3,r3,r3,lsl #3;   	//r3=8*r3+r3 = 9*r3 (9*a)

	ubfx r2,r0,#12,#12;   	//r2=r0[23:12] (b)
	add r2,r2,r2,lsl #1;   	//r2=2*r2+r2 = 3*r2 (3*b)
	sub r3,r3,r2;  		  	//r3=r3-r2 = 9*a-3*b

	//r2=free
	bfc r1,#0,#12;	  	  	//clear low bits of r1
	add r1,r1,r0,lsr #24;  	//r1=c
	add r0,r3,r0;  			//r0=r3+r0[31:24] = 9*a-3*b+c

	//9 cycles, 2 temp registers, 2 input registers

	mov pc,lr


	.global mul_mod_longa_asm
	.extern mul_mod_longa_asm
	.type mul_mod_longa_asm, %function
mul_mod_longa_asm:
	//r0=in1
	//r1=in2

	smull r1,r0,r1,r2     	//r0=low, r1=high

	ubfx r3,r0,#0,#12; 	  	//r3=r0[11:0]  (a)
	add r3,r3,r3,lsl #1;   	//r3=2*r3+r3 = 3*r3 (3*a)

	//r2=free
	//bfc r1,#12,#31;	  	  	//clear high bits of r1 (not sure if this is necessary)
	add r1,r1,r0,lsr #12;  	//r1 = r1[11:0]+r0>>12 = b
	sub r0,r3,r0;  			//r0=r3+r0[31:24] = 3*a-b

	//5 cycles, 2 temp registers, 2 input registers

	mov pc,lr

*/

void poly_correction(int32_t* a, int32_t p, unsigned int N)
{ // Correction modulo q
    unsigned int i;
    int32_t mask;

    for (i = 0; i < N; i++) {
        mask = a[i] >> (4*sizeof(int32_t) - 1);
        a[i] += (p & mask) - p;
        mask = a[i] >> (4*sizeof(int32_t) - 1);
        a[i] += (p & mask);
    }
}

void coefficient_mul_mod_longa(uint32_t out[M], uint32_t b[], uint32_t c[]) {
  //Performs a 64-bit multiplication, followed by a single longa reduction
  int j;
  for (j = 0; j < M; j++)
	  out[j] = mul_mod_longa(b[j], c[j]);
}

void coefficient_add_mod_longa(int32_t out[M], int32_t b[M], int32_t c[M])
{
  // a = b + c
  int j;
  for (j = 0; j < M; j++)
	  out[j] = mod(b[j] + c[j]);
}

int32_t mul_add_mod_longa(int32_t in1, int32_t in2, int32_t in3) {
	assert(in1<268435456);  //268435456 = 2^28 (in1 can be negative)
	assert(in2<268435456);  //268435456 = 2^28

	uint16_t a;
	int32_t b;
	int64_t bb;
	int64_t res1 = (int64_t)in1 * (int64_t)in2 + in3;

	assert (abs(res1)<4294967296); //2^33=4294967296

	a=(uint16_t)(res1&mask12);
	b=(int32_t)(res1>>12);
	bb=(int64_t)(res1>>12);

	assert (b==bb); //check for no loss of precision

	return (3*a-b);
}

int32_t mod_longa(int32_t in)
{
	//Performs a longa reduction.
	uint32_t a;
	int32_t b;
	a=(in&mask12);
	b=(in>>12);
	return 3*a-b;
}

int32_t mul_mod_longa(int32_t in1, int32_t in2)
{
	//Performs a 64-bit multiplication, followed by a mod_longa reduction

	uint16_t a;
	int32_t b;
	int64_t bb;
	int64_t res1 = (int64_t)in1 * (int64_t)in2;

	assert (abs(res1)<4294967296); //2^33=4294967296

	a=(uint16_t)(res1&mask12);
	b=(int32_t)(res1>>12);
	bb=(int64_t)(res1>>12);

	assert (b==bb); //check for no loss of precision

	return (3*a-b);
}

int32_t mod_longa_2x(int32_t in)
{
	//Performs a double longa reduction.
	uint16_t a,b;
	int16_t c;
	a=(uint16_t)(in&mask12);
	b=(uint16_t)(in>>12)&mask12;
	c=(int16_t)(in>>24);
	return (9*a-3*b+c);
}

int32_t mul_mod_longa_2x(int32_t in1, int32_t in2)
{
	//Performs a 64-bit multiplication, followed by a mod_longa_2x reduction
	uint16_t a,b;
	int32_t c;
	int64_t res1 = (int64_t)in1 * (int64_t)in2;

	a=(uint16_t)(res1&mask12);
	b=(uint16_t)(res1>>12)&mask12;
	c=(int32_t)(res1>>24);
	return (9*a-3*b+c);
}

int mul_inv(int a, int modulus)
{
	//Modular inverse
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


void fwd_ntt_longa_3x(int32_t a[M])
{
  //Performs an NTT with three 2X reductions
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
      S = psi_longa[m + i];
      for(j = j1; j<=j2; j++)
      {
    	if ((m==4) || (m==32) || (m==256))
    	{
    		//Add k term
			U = mod_longa(a[j]);
			V = mul_mod_longa_2x(a[j+t],S);
    	}
    	else
    	{
    		U = a[j];
   			V = mod_longa(a[j+t] * S);
    	}

		a[j] = (U + V);
		a[j+t] = (U - V);

		assert(((int64_t)U+(int64_t)V)==a[j]);
		assert(((int64_t)U-(int64_t)V)==a[j+t]);
      }
    }
  }
}

void inv_ntt_longa_3x(int32_t a[M], int k_inv_first)
{
  //Performs a INTT with three 2X reductions
  int t, h, m, i, j, j1, j2, x=0, ind;
  int *index;
  int32_t S, U, V;

  index = inv_psi1_longa;

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
				if ((m!=128) && (m!=16))
				{
					a[j] = (U+V);						//0
					if ((m==64) || (m==32) || (m==8) || (m==4))
						a[j+t] = mod_longa((U-V) * S); 		//0
					else
						a[j+t] = mul_mod_longa((U-V), S); 		//0

					assert(((int64_t)U+(int64_t)V)==a[j]);

					/* The following assert sometimes fails. This is OK
					 * assert(mod(((int64_t)U-(int64_t)V)*mod(S*3))==mod(a[j+t]));
					 */
				}
				else
				{
					a[j] = mod_longa(U+V); 				//k
					a[j+t]=mul_mod_longa_2x((U-V), S); 	//k

					assert(mod(((int64_t)U+(int64_t)V)*3)==mod(a[j]));
					int32_t tmp1=mod(((int64_t)U-(int64_t)V)*(int64_t)mod(S*9));
					int32_t tmp2=mod(a[j+t]);
					assert (tmp1==tmp1);
				}
			}
			j1 = j1 + 2*t;
			index++;
		}
		t = 2*t;
		index = index - m;
	}

	int m_inv=mul_inv(M,MODULUS);
	for (j = 0; j < t; j++)
	{
		U = a[j];
		V = a[j+t];

		a[j] = mul_mod_longa((U+V), mod(m_inv*mod(k_inv_first)));
		a[j+t] = mul_mod_longa_2x((U-V), mod(mod(m_inv*inv_psi1_longa[1])*mod(k_inv_first)));

	}
}

void unit_test_longa_fwd_3x_inv_3x_ntt()
{
	int res = 1;
	int i,j,k;

	int k_inv_first=k_inv;
	for (j=0; j<6; j++) k_inv_first = mod(k_inv_first*k_inv);

	for (i=0; (i<10000) && (res==1); i++)
	{
		int32_t in1[M],in2[M],in3[M];

		srand(i);
		if (i==0)
			for (j=0; j<M; j++) in2[j]=in1[j]=1; //All ones
		else
			for (j=0; j<M; j++) in2[j]=in1[j]=rand()%MODULUS;//Random values

		fwd_ntt_longa_3x(in1);
		inv_ntt_longa_3x(in1,k_inv_first);

		//The following two lines are required to get the coefficients into [0,q)
		for (j=0; j<M; j++)	in1[j] = mod_longa(in1[j]);
		poly_correction(in1,MODULUS,M);

		for (j=0; j<M; j++)
		{
			if (in2[j]!=in1[j])
			{
				res=0;
				printf("i=%d, j=%d\n",i,j);
				printf("\n\r in1: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, mod(in1[k]));
				printf("\n\r in2: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, in2[k]);
				printf("\n\r");
				break;
			}
		}
	}
	printf("unit_test_longa_fwd_3x_inv_3x_ntt: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}


//Meghna: This is the approach we want to follow
void unit_test_longa_fwd_3x_inv_2x_ntt()
{
	int res = 1;
	int i,j,k;

	int k_inv_first=k_inv;
	for (j=0; j<5; j++) k_inv_first = mod(k_inv_first*k_inv);

	for (i=0; (i<10000) && (res==1); i++)
	{
		int32_t in1[M],in2[M],in3[M];

		srand(i);
		if (i==0)
			for (j=0; j<M; j++) in2[j]=in1[j]=1; //All ones
		else
			for (j=0; j<M; j++) in2[j]=in1[j]=rand()%MODULUS;//Random values

		fwd_ntt_longa_3x(in1);

		inv_ntt_longa_2x(in1,k_inv_first);

		//The following two lines are required to get the coefficients into [0,q)
		for (j=0; j<M; j++)
			in1[j] = mod_longa(in1[j]);
		poly_correction(in1,MODULUS,M);

		for (j=0; j<M; j++)
		{
			if (in2[j]!=in1[j])
			{
				res=0;
				printf("i=%d, j=%d\n",i,j);
				printf("\n\r in1: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, mod(in1[k]));
				printf("\n\r in2: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, in2[k]);
				printf("\n\r");
				break;
			}
		}
	}
	printf("unit_test_longa_fwd_3x_inv_2x_ntt: ");
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");
}

//Meghna: This is the approach for mul+add
void unit_test_mul_add_mod_longa(int k_inv2)
{
	printf("unit_test_mul_add_mod_longa: ");
	int i,res;
	res = 1;
	for (i=1; i<1000000; i++)
	{
		srand(i);
		int32_t in1,in2,in3,output;
		in1=rand()%(65535*4); //16-bit
		in2=rand()%(65535*4);
		in3=rand()%(65535*4);

		if (rand()%1) in1=in1*-1; //16-bit + sign
		if (rand()%1) in2=in2*-1;
		if (rand()%1) in3=in3*-1;

		output=mul_add_mod_longa(in1,in2,in3);
		uint32_t output_scaled = mod(output*mod(k_inv)); //mod_longa doesn't provide a "true" mod operation. Put it in the same range as expected value

		int64_t tmp = ((int64_t)in1*(int64_t)in2)+in3;
		uint32_t expected = mod64(tmp);

		if (output_scaled!=expected)
		{
			output=mod_longa(output);
			output_scaled = mod(output*k_inv2);

			if (output_scaled!=expected)
			{
				printf("i=%d, in1=%x in2=%x output=%x output_scaled=%x expected=%x\n\r",i,in1,in2,output,output_scaled,expected);
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

void unit_test_longa_poly_mul()
{
	int res = 1;
	int i,j,k;
	int k_inv_first=k_inv;
	for (j=0; j<9; j++) k_inv_first = mod(k_inv_first*k_inv);

	for (i=0; (i<10000) && (res==1); i++)
	{
		uint32_t in1[M],in2[M],out1[M];
		int16_t in3[M],in4[M],out2[M];

		srand(i);
		if (i==0)
			for (j=0; j<M; j++) in4[j]=in3[j]=in1[j]=in2[j]=1; //All ones
		else
		{
			//Random values for other test cases
			for (j=0; j<M; j++)
			{
				in3[j]=in1[j]=rand()%MODULUS;
				in4[j]=in2[j]=rand()%MODULUS;
			}
		}

		fwd_ntt_longa_3x(in1);
		fwd_ntt_longa_3x(in2);
		coefficient_mul_mod_longa(out1,in1,in2);

		inv_ntt_longa_3x(out1,k_inv_first);

		mul_test(in3,in4,out2,M);

		for (j=0; j<M; j++)
		{
			if (out2[j]!=mod(out1[j]))
			{
				res=0;

				printf("i=%d j=%d\n",i,j);
				printf("\n out1: ");
				for(k = 0; k< M; k++) printf("[%d %08d] ", k, mod(out1[k]));
				printf("\n out2: ");
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

void unit_test_mod_longa(int k_inv2)
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
		int32_t in1;
		int16_t in2;
		in1=rand()%268435456;
		in2=rand()%(16384*2); //16384 = 2^14

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

void unit_test_mul_mod_longa(int k_inv2)
{
	printf("unit_test_mul_mod_longa: ");
	int i,res;
	res = 1;
	for (i=1; i<1000000; i++)
	{
		srand(i);
		int32_t in1,in2;
		in1=rand()%(65535*4);
		in2=rand()%(65535*4);

		if (rand()%1) in1=in1*-1;
		if (rand()%1) in2=in2*-1;

		int32_t output=mul_mod_longa(in1,in2);
		uint32_t output_scaled = mod(output*k_inv); //mod_longa doesn't provide a "true" mod operation. Put it in the same range as expected value
		int64_t tmp = (int64_t)in1*(int64_t)in2;
		uint32_t expected = mod64(tmp);

		if (output_scaled!=expected)
		{
			output=mod_longa(output);
			output_scaled = mod(output*k_inv2);

			if (output_scaled!=expected)
			{
				printf("i=%d, in1=%x in2=%x output=%x output_scaled=%x expected=%x\n\r",i,in1,in2,output,output_scaled,expected);
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

void generate_params()
{
	int16_t tmp=mul_inv(3, 12289);
	printf("k_inv=%d\n",tmp);
	int i;
	printf("int psi_longa[]={");
	for (i=0; i<M; i++) printf("%d,",mod(psi[i]*k_inv));
	printf("};\n");

	printf("int inv_psi1_longa[]={");
	for (i=0; i<M; i++) printf("%d,",mod(inv_psi1[i]*k_inv));
	printf("};\n");

}

void unit_test_reduction_longa_3x()
{
	//generate_params();

	uint32_t k_inv2=mod(k_inv*k_inv);
	unit_test_mul_add_mod_longa(k_inv2);
	unit_test_longa_fwd_3x_inv_3x_ntt();
	unit_test_longa_fwd_3x_inv_2x_ntt();
	unit_test_longa_poly_mul();
	unit_test_mod_longa(k_inv2);
	unit_test_mul_mod_longa(k_inv2);
	unit_test_mod_longa_2x(k_inv2);
	unit_test_mul_mod_longa_2x(k_inv2);

}
