/*****************************************************************************
* Copyright 2014 Katholieke Universiteit Leuven
*
* Use of this software is governed by the GNU LGPLv3.0 license
*
* Written by Ruan de Clercq, Sujoy Sinha Roy,
* Frederik Vercauteren, and Ingrid Verbauwhede
* Computer Security and Industrial Cryptography (COSIC)
* K.U.Leuven, Departement Electrical Engineering,
* Celestijnenlaan 200A, B-3001 Leuven, Belgium
****************************************************************************/

#include "lwe.h"
#include "luts.h"
#include "stdlib.h"
#include "global.h"

#ifdef USE_FAKE_GET_RAND
uint32_t g_fake_rand;
#endif

#ifdef USE_FAKE_GET_RAND
uint32_t get_rand()
{
	return g_fake_rand;
}
#else
uint32_t get_rand()
{
	uint32_t rnd = rand();
	rnd |= 0x80000000; //set the least significant bit
	return rnd;
}
#endif

uint32_t clz(uint32_t a)
{
	int i;
	for (i=0; i<32; i++)
	{
		if ((a>>(31-i))==1)
		{
			return i;
		}
	}
	return 32;
}

uint32_t mod(int a)
{
	int quotient, remainder;
	quotient = a/MODULUS;
	if(a>=0)
		remainder = a - quotient*MODULUS;
	else
		remainder = (1-quotient)*MODULUS + a;

	return(remainder);
}


void knuth_yao_basic_array(uint32_t a[M])
{
	int i;
	uint32_t rnd;
	rnd=get_rand();
	for (i=0; i<M/2; i++)
	{
#ifdef DISABLE_KNUTH_YAO
		a[2*i+1]=0;
		a[2*i]=0;
#else
		a[2*i+1]=knuth_yao_basic_single_number(&rnd);
		a[2*i]=knuth_yao_basic_single_number(&rnd);
#endif
	}
}

uint32_t knuth_yao_basic_single_number(uint32_t * rnd)
{
	int distance;
	int row, column;
	int index, sample, sample_msb;

#ifdef DEBUG_PRINTF
	printf("Start rnd:%d\n",*rnd);
#endif

	//Real knuth-yao
	distance = 0;
	for(column=0; (column<PMAT_MAX_COL); column++)
	{
		distance = distance*2 + ((*rnd)&1);
		(*rnd) = (*rnd)>>1;
		if ((*rnd)==NEW_RND_BOTTOM)
		{
			(*rnd)=get_rand();
		}
#ifdef DEBUG_PRINTF
		printf("rnd:%d, dist:%d, col:%d\n",(*rnd),distance,column);
#endif

		// Read probability-column 0 and count the number of non-zeros
		for(row=PMAT_MAX_ROW; row>=0; row--)
		{
			distance = distance - pmat[row][column];
			if(distance<0)
			{
#ifdef DEBUG_PRINTF
				printf("rnd:%d",(*rnd));
#endif
				if ((*rnd)&1)
					sample = (MODULUS - row);
				else
					sample = row;
				(*rnd) = (*rnd) >> 1;
				if (clz(*rnd)>(NEW_RND_LARGE))
				{
					(*rnd)=get_rand();
				}

				return sample;
			}
		}
	}

	return -1;
}


void knuth_yao_lut_optimized_array( int a[M])
{
	int i;
	uint32_t rnd;
	rnd=get_rand();
	for (i=0; i<M; i++)
	{
		a[i]=knuth_yao_single_number_lut_optimized(&rnd);
	}
}

uint32_t knuth_yao_single_number_lut_optimized(uint32_t * rnd)
{
	int distance;
	int row, column;
	int index, sample, sample_msb;


#ifdef DEBUG_PRINTF
	printf("Start rnd:%d\n",*rnd);
#endif
	index = (*rnd)&0xff;
	(*rnd) = (*rnd) >> 8;

	sample = lut1[index]; //M elements in lut1
	sample_msb = sample & 16;
	if(sample_msb==0)	  //lookup was successful
	{
		if ((*rnd)==NEW_RND_BOTTOM)
		{
			(*rnd)=get_rand();
		}
		sample = sample & 0xf;
		if ((*rnd)&1) sample = (MODULUS - sample); //9th bit in rnd is the sign
		(*rnd) = (*rnd) >> 1;
		//We know that in the next call we will need 8 bits!
		if (clz(*rnd)>(NEW_RND_LARGE))
		{
			(*rnd)=get_rand();
		}

		return sample;
	}
	else
	{
		if (clz(*rnd)>(NEW_RND_MID))
		{
			(*rnd)=get_rand();
		}
		distance = sample & KN_DISTANCE1_MASK;
		index = ((*rnd)&0x1f) + 32*distance;
		(*rnd) = (*rnd)>>5;
		if ((*rnd)==NEW_RND_BOTTOM)
		{
			(*rnd)=get_rand();
		}
		sample = lut2[index]; //224 elements in lut2
		sample_msb = sample & 32;
		if(sample_msb==0)	// lookup was successful
		{
			sample = sample & 31;
			if ((*rnd)&1) sample = (MODULUS - sample); //9th bit in rnd is the sign
			(*rnd)=(*rnd)>>1;
			if (clz(*rnd)>(NEW_RND_LARGE))
			{
				(*rnd)=get_rand();
			}
			return sample;
		}
		else
		{
			//Real knuth-yao
			distance = sample & KN_DISTANCE2_MASK;
			for(column=13; (column<PMAT_MAX_COL); column++)
			{
				distance = distance*2 + ((*rnd)&1);
				(*rnd) = (*rnd)>>1;
				if ((*rnd)==NEW_RND_BOTTOM)
				{
					(*rnd)=get_rand();
				}
#ifdef DEBUG_PRINTF
				printf("rnd:%d, dist:%d, col:%d\n",(*rnd),distance,column);
#endif

				// Read probability-column 0 and count the number of non-zeros
				for(row=PMAT_MAX_ROW; row>=0; row--)
				{
					distance = distance - pmat[row][column];
					if(distance<0)
					{
#ifdef DEBUG_PRINTF
						printf("rnd:%d",(*rnd));
#endif
						if ((*rnd)&1)
							sample = (MODULUS - row);
						else
							sample = row;
						(*rnd) = (*rnd) >> 1;
						if (clz(*rnd)>(NEW_RND_LARGE))
						{
							(*rnd)=get_rand();
						}

						return sample;
					}
				}
			}
		}
	}

	return -1;
}

void a_gen2(uint32_t a[])
{
	int i, r;

	for(i=0;i<M/2;i++)
	{
		r = rand();
		a[2*i] = mod(r&0xffff);
		a[2*i+1] = mod((r>>16));
	}

	fwd_ntt2(a);
}


void r1_gen2(uint32_t r1[])
{
	knuth_yao_basic_array(r1);
	fwd_ntt2(r1);
}

void r2_gen2(uint32_t r2[M])
{
	int i, j, r, bit, sign;

	for(i=0;i<M;)
	{
		r = rand();

		for (j=0; j<16; j++)
		{
			bit = r&1;
			sign = (r>>1)&1;
			if (sign==1 && bit==1) bit = (MODULUS-1);
			r2[i++] = bit;
			r=r>>2;
		}
	}
	fwd_ntt2(r2);
}
/*
 * void r2_gen2(int r2[M])
{
	int i, j, r, bit, sign;

	for(i=0;i<M;)
	{
		r = get_rand();

		for (j=0; j<16; j++)
		{
			bit = r&1;
			sign = (r>>1)&1;
			if (sign==1 && bit==1) bit = 7680;
			r2[i++] = bit;
			r=r>>2;
		}
	}
	//fwd_ntt2(r2);
}
 */


void rearrange(uint32_t a[M])
{
	int u1, t1, u2, t2, i, bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8, swp_index;

	for(i=1; i<M/2; i++)
	{
		bit1 = i%2;
		bit2 = (i>>1)%2;
		bit3 = (i>>2)%2;
		bit4 = (i>>3)%2;
		bit5 = (i>>4)%2;
		bit6 = (i>>5)%2;
		bit7 = (i>>6)%2;

#ifdef NTT512
		int bit8 = (i>>7)%2;
		swp_index = bit1*128 + bit2*64 + bit3*32 + bit4*16 + bit5*8 + bit6*4 + bit7*2 + bit8;
#else
		swp_index = bit1*64 + bit2*32 + bit3*16 + bit4*8 + bit5*4 + bit6*2 + bit7;
#endif

		if(swp_index>i)
		{
			u1 = a[2*i];
			u2 = a[2*i+1];

			a[2*i] = a[2*swp_index];
			a[2*i+1] = a[2*swp_index+1];

			a[2*swp_index] = u1;
			a[2*swp_index+1] = u2;
		}
	}
}

void bitreverse2(uint32_t a[M])
{
	int i, swp_index, temp;
	char bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8;
	int q1, r1, q2, r2;

	for(i=0; i<M; i++)
	{
		bit1 = i%2;
		bit2 = (i>>1)%2;
		bit3 = (i>>2)%2;
		bit4 = (i>>3)%2;
		bit5 = (i>>4)%2;
		bit6 = (i>>5)%2;
		bit7 = (i>>6)%2;
		bit8 = (i>>7)%2;

#ifdef NTT512
		int bit9 = (i>>8)%2;
		swp_index = bit1*256 + bit2*128 + bit3*64 + bit4*32 + bit5*16 + bit6*8 + bit7*4 + bit8*2 + bit9;
#else
		swp_index = bit1*128 + bit2*64 + bit3*32 + bit4*16 + bit5*8 + bit6*4 + bit7*2 + bit8;
#endif
		q1=i/2;
		r1=i%2;
		q2 = swp_index/2;
		r2 = swp_index%2;

		if(swp_index>i)
		{
			if(r2==0) temp = a[2*q2];
			if(r2==1) temp = a[2*q2+1];
			if(r2==0 && r1==0) a[2*q2] = a[2*q1];
			if(r2==0 && r1==1) a[2*q2] = a[2*q1+1];
			if(r2==1 && r1==0) a[2*q2+1] = a[2*q1];
			if(r2==1 && r1==1) a[2*q2+1] = a[2*q1+1];
			if(r1==0) a[2*q1] = temp;
			if(r1==1) a[2*q1+1] = temp;
		}
	}
}

void fwd_ntt2(uint32_t a[])
{
	int i, j, k, m;
	int u1, t1, u2, t2;
	int primrt, omega;


	i=0;
	for(m=2; m<=M/2; m=2*m)
	{
		primrt=primitive_root_table[i];
		omega=primitive_root_table[i+1];
		i++;

		for(j=0; j<m; j+=2)
		{
			for(k=0; k<M; k=k+2*m)
			{
				u1 = a[j+k];
				t1 = mod(omega * a[j+k+1]);

				u2 = a[j+k+m];
				t2 = mod(omega * a[j+k+m+1]);


				a[j+k] = mod(u1 + t1);
				a[j+k+1] = mod(u2 + t2);

				a[j+k+m] = mod(u1 - t1);
				a[j+k+m+1] = mod(u2 - t2);
			}
			omega = omega * primrt;
			omega = mod(omega);
		}
	}

	primrt = FWD_CONST1;
	omega = FWD_CONST2;
	for(j=0; j<M/2; j++)
	{
		t1 = omega * a[2*j+1];
		t1 = mod(t1);
		u1 = a[2*j];
		a[2*j] = u1 + t1;
		a[2*j] = mod(a[2*j]);
		a[2*j+1] = u1 - t1;
		a[2*j+1] = mod(a[2*j+1]);

		omega = omega * primrt;
		omega = mod(omega);
	}
}


void inv_ntt(uint32_t a[M])
{
	int i, j, k, m, u1, t1, u2, t2, primrt, omega;

	i=0;
	for(m=2; m<=M/2; m=2*m)
	{
#ifdef NTT512
		switch (m)
		{
			case 2: primrt=12288;
					break;
			case 4: primrt=10810;
					break;
			case 8: primrt=7143;
					break;
			case 16:primrt=10984;
					break;
			case 32:primrt=3542;
					break;
			case 64:primrt=4821;
					break;
			case 128:primrt=1170;
					break;
			case 256:primrt=5755;
					break;
		}
#else
		switch (m)
		{
			case 2: primrt=7680;
					break;
			case 4: primrt=3383;
					break;
			case 8: primrt=5756;
					break;
			case 16:primrt=1728;
					break;
			case 32:primrt=7584;
					break;
			case 64:primrt=6569;
					break;
			case 128:primrt=6601;
					break;
		}
#endif

		primrt = primrt_inv_omega_table[i];
		i++;
		omega = 1;
		for(j=0; j<m/2; j++)
		{
			for(k=0; k<M/2; k=k+m)
			{
				t1 = omega * a[2*(k+j)+1];
				t1 = mod(t1);
				u1 = a[2*(k+j)];
				t2 = omega * a[2*(k+j+m/2)+1];
				t2 = mod(t2);
				u2 = a[2*(k+j+m/2)];

				a[2*(k+j)] = u1 + t1;
				a[2*(k+j)] = mod(a[2*(k+j)]);
				a[2*(k+j+m/2)] = u1 - t1;
				a[2*(k+j+m/2)] = mod(a[2*(k+j+m/2)]);

				a[2*(k+j)+1] = u2 + t2;
				a[2*(k+j)+1] = mod(a[2*(k+j)+1]);
				a[2*(k+j+m/2)+1] = u2 - t2;
				a[2*(k+j+m/2)+1] = mod(a[2*(k+j+m/2)+1]);
			}
			omega = omega * primrt;
			omega = mod(omega);
		}
	}

	primrt = INVCONST1;
	omega = 1;
	for(j=0; j<M;)
	{
		u1 = a[j];
		j++;
		t1 = omega * a[j];
		t1 = mod(t1);

		a[j-1] = u1 + t1;
		a[j-1] = mod(a[j-1]);
		a[j] = u1 - t1;
		a[j] = mod(a[j]);
		j++;

		omega = omega * primrt;
		omega = mod(omega);
	}
	int omega2 = INVCONST2;
	primrt = INVCONST3;
	omega = 1;

	for(j=0; j<M;)
	{
		a[j] = omega * a[j];
		a[j] = mod(a[j]);

		a[j] = a[j]*SCALING;
		a[j] = mod(a[j]);

		j++;
		a[j] = omega2 * a[j];
		a[j] = mod(a[j]);

		a[j] = a[j]*SCALING;
		a[j] = mod(a[j]);
		j++;

		omega = omega * primrt;
		omega = mod(omega);
		omega2 = omega2 * primrt;
		omega2 = mod(omega2);
	}
	/*
	omega = 1;
	for(j=0; j<M; j++)
	{
		a[j] = a[j] * 7651;
		a[j] = mod(a[j]);
	}*/
}

void coefficient_add(uint32_t a_0[], uint32_t a_1[], uint32_t b_0[], uint32_t b_1[])
{
	//a = a + b
	int i, j, k;
	int u1, t1, u2, t2;

	for(j=0; j<M/2; j++)
	{
		a_0[j] = a_0[j] + b_0[j];
		a_0[j] = mod(a_0[j]);
		a_1[j] = a_1[j] + b_1[j];
		a_1[j] = mod(a_1[j]);
	}
}

void coefficient_mul(uint32_t a_0[], uint32_t a_1[], uint32_t b_0[], uint32_t b_1[])
{
	//a = a * b
	int i, j, k;
	int u1, t1, u2, t2;

	for(j=0; j<M/2; j++)
	{
		a_0[j] = a_0[j] * b_0[j];
		a_0[j] = mod(a_0[j]);
		a_1[j] = a_1[j] * b_1[j];
		a_1[j] = mod(a_1[j]);
	}
}

void coefficient_sub(uint32_t a_0[], uint32_t a_1[], uint32_t b_0[], uint32_t b_1[])
{
	int i, j, k, m;
	int u1, t1, u2, t2;

	for(j=0; j<M/2; j++)
	{
		a_0[j] = a_0[j] - b_0[j];
		a_0[j] = mod(a_0[j]);
		a_1[j] = a_1[j] - b_1[j];
		a_1[j] = mod(a_1[j]);
	}
}


uint32_t compare_large_simd(uint32_t large_simd[M/2],uint32_t large[M])
{
	int j;
	for (j=0; j<M/2; j++)
	{
		if (((large_simd[j]&0xffff)!=large[2*j]))
		{
			//xprintf("(j_low=%x)",j);
			return 0;
			break;
		}

		if ((large_simd[j]>>16)!=large[2*j+1])
		{
			//xprintf("(j_high=%x)",j);
			return 0;
			break;
		}
	}

	return 1;
}


void coefficient_mul2(uint32_t a[M], uint32_t b[], uint32_t c[])
{
	//a = b * c
	int j;

	for(j=0; j<M; j++)
	{
		a[j] = b[j] * c[j];
		a[j] = mod(a[j]);
	}
}


void coefficient_add2(uint32_t a[M], uint32_t b[M], uint32_t c[M])
{
	//a = b + c
	int j;

	for(j=0; j<M; j++)
	{
		a[j] = b[j] + c[j];
		a[j] = mod(a[j]);
	}
}

void coefficient_mul_add2(uint32_t * result, uint32_t * large1, uint32_t * large2, uint32_t * large3)
{
	//result=large1*large2+large3
	int j;

	for(j=0; j<M; j++)
	{
		result[j] = large1[j] * large2[j];
		result[j] = result[j] + large3[j];
		result[j] = mod(result[j]);
	}
}

void coefficient_sub2(uint32_t a[M], uint32_t b[M], uint32_t c[M])
{
	int j;

	for(j=0; j<M; j++)
	{
		a[j] = b[j] - c[j];
		a[j] = mod(a[j]);
	}
}


void key_gen(uint32_t a[M], uint32_t p[M], uint32_t r2[M])
{
	a_gen2(a);
	r1_gen2(p);
	r2_gen2(r2);

	uint32_t tmp_a[M]; 					//Temporary storage for a*r2
	coefficient_mul2(tmp_a, a, r2); 	//a = a*r2
	coefficient_sub2(p, p, tmp_a); 		//p = p-a*r2
	rearrange(r2);
}

void rlwe_enc(uint32_t a[M], uint32_t c1[M], uint32_t c2[M], uint32_t m[M], uint32_t p[M])
{
	int i;
	uint32_t e1[M], e2[M], e3[M], encoded_m[M];

	for(i=0; i<M; i++)
	{
		encoded_m[i] = m[i] * QBY2; 		//Encode the message
	}

	knuth_yao_basic_array(e1);
	knuth_yao_basic_array(e2);
	knuth_yao_basic_array(e3);

	coefficient_add2(e3, e3, encoded_m);	// e3 <-- e3 + m

	fwd_ntt2(e1);
	fwd_ntt2(e2);
	fwd_ntt2(e3);

	// m <-- a*e1
	coefficient_mul2(c1,a,e1); 				// c1 <-- a*e1
	coefficient_add2(c1,e2, c1);			// c1 <-- e2 + a*e1(tmp_m);
	coefficient_mul2(c2,p,e1); 				// c2 <-- p*e1
	coefficient_add2(c2, e3, c2);			// c2<-- e3 + p*e1

	rearrange(c1);
	rearrange(c2);
}

void rlwe_dec(uint32_t c1[M], uint32_t c2[M], uint32_t r2[M])
{
	coefficient_mul2(c1, c1, r2);	// c1 <-- c1*r2
	coefficient_add2(c1, c1, c2);	// c1 <-- c1*r2 + c2

	inv_ntt(c1);
}

void message_gen(uint32_t m[M])
{
	int i;
	for(i=0;i<M;i++)
	{
		m[i] = get_rand()%2;
	}
}

void rearrange_decrypted_message(uint32_t in[M],uint32_t out[M])
{
	int i;
	for (i=0; i<M/2; i+=2)
	{
		out[i]=in[2*i];
		out[i+1]=in[2*(i+1)];
	}

	for (i=0; i<M/2; i+=2)
	{
		out[i+M/2]=in[2*i+1];
		out[i+1+M/2]=in[2*(i+1)+1];
	}
}

void get_rand_input(int i, uint32_t large1[M],uint32_t large2[M])
{
	int rnd1,rnd2,j;
	srand(i);
	if (i==0)
	{
		for (j=0; j<M/2; j++)
		{
			rnd1=2*j;
			rnd2=2*j+1;
			large1[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
			large2[2*j]=rnd1;
			large2[2*j+1]=rnd2;
		}
	}
	else
	{
		for (j=0; j<M/2; j++)
		{
			rnd1=get_rand()&COEFFICIENT_ALL_ONES;
			rnd2=get_rand()&COEFFICIENT_ALL_ONES;
			large1[j]=(rnd1&0xffff)+((rnd2&0xffff)<<16);
			large2[2*j]=rnd1;
			large2[2*j+1]=rnd2;
		}
	}
}
