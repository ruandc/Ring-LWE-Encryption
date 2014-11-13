#include "global.h"
#include "stdint.h"
#include "test_asm.h"
#include "knuth_yao_asm.h"
#include "stdlib.h"
#include "string.h"


void r1_gen_asm(uint32_t * a)
{
	knuth_yao_asm(a);
	fwd_ntt_asm(a);
}

void key_gen_asm(uint32_t * large1, uint32_t * large2, uint32_t * large3)
{
	a_gen_asm(large1);
	r1_gen_asm(large2);
	r2_gen_asm(large3);

	uint32_t tmp_a[M/2];
	coefficient_mul_asm(tmp_a, large1, large3);
	coefficient_sub_asm(large2, large2, tmp_a);
	rearrange_asm(large3);
}

void RLWE_dec_asm(uint32_t * c1, uint32_t * c2, uint32_t * r2)
{
	//coefficient_mul_asm(c1,c1,r2); //coefficient_mul2(c1, c1, r2);	// c1 <-- c1*r2
	//coefficient_add_asm(c1,c1,c2); //coefficient_add2(c1, c1, c2);	// c1 <-- c1*r2 + c2

	coefficient_mul_add_asm(c1,c1,r2,c2);

	inv_ntt_asm(c1); //inv_ntt2(c1);
}

void RLWE_enc_asm(uint32_t * a, uint32_t * c1, uint32_t * c2, uint32_t * m, uint32_t * p)
{
	int i;

	uint32_t encoded_m[M/2];

	encode_message_asm(encoded_m,m); //for(i=0; i<M/2; i++) m[i] = ((m[i]&0xffff) * QBY2) + ((((m[i]>>16)&0xffff) * QBY2)<<16);

#ifdef USE_PARALLEL
	knuth_yao_asm(fixed_data1);
	knuth_yao_asm(fixed_data2);
	knuth_yao_asm(fixed_data3);

	coefficient_add_asm(fixed_data3, fixed_data3, encoded_m);	// e3 <-- e3 + m

	fwd_ntt_parallel_asm(fixed_data1); //Uses fixed_data1+fixed_data2+fixed_data3. It knows that there is an offset of 0x200 between each consecutive item

	// m <-- a*e1
	//coefficient_mul_asm(tmp_m,a,fixed_data1); 		//tmp_m <-- a*e1
	//coefficient_add_asm(c1, fixed_data2, tmp_m);	//c1 <-- e2 <-- e2 + a*e1(tmp_m);
	coefficient_mul_add_asm(c1,a,fixed_data1,fixed_data2);
	//coefficient_mul_asm(tmp_m,p,fixed_data1); 		//tmp_m <-- p*e1
	//coefficient_add_asm(c2, fixed_data3, tmp_m);	//c2 <-- e3 + p*e1
	coefficient_mul_add_asm(c2,p,fixed_data1,fixed_data3);

#else
	uint32_t e1[M], e2[M], e3[M], tmp_m[M/2];;
	knuth_yao_asm(e1);
	knuth_yao_asm(e2);
	knuth_yao_asm(e3);

	coefficient_add_asm(e3, e3, encoded_m);	// e3 <-- e3 + m

	fwd_ntt_asm(e1);
	fwd_ntt_asm(e2);
	fwd_ntt_asm(e3);

	coefficient_mul_asm(tmp_m,a,e1); 		//tmp_m <-- a*e1
	coefficient_add_asm(c1, e2, tmp_m);	// c1 <-- e2 <-- e2 + a*e1(tmp_m);
	coefficient_mul_asm(tmp_m,p,e1); 		// tmp_mm <-- p*e1
	coefficient_add_asm(c2, e3, tmp_m);	// c2< <-- e3 <-- e3 + p*e1
#endif

	rearrange_asm(c1);
	rearrange_asm(c2);
}

void message_gen_asm(uint32_t * m)
{
	int i;
	for(i=0;i<M/2;i++)
	{
		m[i] = rand()%2;
		m[i] = m[i]+((rand()%2)<<16);
	}
}

void rearrange_for_final_test_asm(uint32_t in[M/2],uint32_t out[M/2])
{
	int i;
	/*
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
	*/
	for (i=0; i<M/2; i+=2)
	{
		out[i/2]=(in[i]&0xffff)+((in[i+1]&0xffff)<<16);
	}
	for (i=0; i<M/2; i+=2)
	{
		out[i/2+M/4]=((in[i]>>16)&0xffff)+(((in[i+1]>>16)&0xffff)<<16);
	}
}

void insert_lowval(uint32_t * a, uint32_t b)
{
	*a = ((b)&0xffff) + ((*a)&0xffff0000);
}

void insert_highval(uint32_t * a, uint32_t b)
{
	*a = ((b)<<16) + ((*a)&0xffff);
}

void bitreverse_asm(uint32_t a[M])
{
	int i;
	int bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8, bit9, swp_index;
	int q1, r1, q2, r2;
	int temp;

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

		q1 = i/2;
		r1 = i%2;
#ifdef NTT512
		int bit9 = (i>>8)%2;
		swp_index = bit1*256 + bit2*128 + bit3*64 + bit4*32 + bit5*16 + bit6*8 + bit7*4 + bit8*2 + bit9;
#else
		swp_index = bit1*128 + bit2*64 + bit3*32 + bit4*16 + bit5*8 + bit6*4 + bit7*2 + bit8;
#endif
		q2 = swp_index/2;
		r2 = swp_index%2;

		if(swp_index>i)
		{
			if(r2==0) temp = a[q2]&0xffff;
			if(r2==1) temp = (a[q2]>>16)&0xffff;
			if(r2==0 && r1==0) insert_lowval(&a[q2],a[q1]);//a[2*q2] = a[2*q1];
			if(r2==0 && r1==1) insert_lowval(&a[q2],(a[q1]>>16));//a[2*q2] = a[2*q1+1];
			if(r2==1 && r1==0) insert_highval(&a[q2],a[q1]);//a[2*q2+1] = a[2*q1];
			if(r2==1 && r1==1) insert_highval(&a[q2],(a[q1]>>16));//a[2*q2+1] = a[2*q1+1];
			if(r1==0) insert_lowval(&a[q1],temp);//a[2*q1] = temp;
			if(r1==1) insert_highval(&a[q1],temp);//a[2*q1+1] = temp;
		}
	}
}

void ntt_multiply_asm(uint32_t * a, uint32_t * b, uint32_t * c)
{
	fwd_ntt_asm(a);
	fwd_ntt_asm(b);

	coefficient_mul_asm(c,a,b);
	rearrange_asm(c);
	inv_ntt_asm(c);
/*
	fwd_ntt2(large2);
	rearrange2(large2);
	inv_ntt2(large2);
	*/
}
