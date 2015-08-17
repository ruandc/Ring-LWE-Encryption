#include "lwe_original.h"
#include "stdio.h"

void fwd_ntt(int a_0[], int a_1[])
{
	int i, j, k, m;
	int u1, t1, u2, t2;
	int primrt, omega;

	//primitive root = 5118
	//square_root = 1065

	for(m=2; m<=M/2; m=2*m)
	{

		if(m == 2) primrt=7680;		//mpz_set_str(primrt,"7680",10);
		if(m == 4) primrt=4298;		//mpz_set_str(primrt,"4298",10);
		if(m == 8) primrt=6468;		//mpz_set_str(primrt,"6468",10);
		if(m == 16) primrt=849;		//mpz_set_str(primrt,"849",10);
		if(m == 32) primrt=2138;	//mpz_set_str(primrt,"2138",10);
		if(m == 64) primrt=3654;	//mpz_set_str(primrt,"3654",10);
		if(m == 128) primrt=1714;	//mpz_set_str(primrt,"1714",10);

		if(m == 2) omega = 4298; 	//mpz_set_str(omega,"4298",10);
		if(m == 4) omega = 6468;	//mpz_set_str(omega,"6468",10);
		if(m == 8) omega = 849;		//mpz_set_str(omega,"849",10);
		if(m == 16) omega = 2138;	//mpz_set_str(omega,"2138",10);
		if(m == 32) omega = 3654;	//mpz_set_str(omega,"3654",10);
		if(m == 64) omega = 1714;	//mpz_set_str(omega,"1714",10);
		if(m == 128) omega = 5118;	//mpz_set_str(omega,"5118",10);


		for(j=0; j<m/2; j++)
		{
			for(k=0; k<128; k=k+m)
			{
				t1 = omega * a_1[k+j];
				t1 = mod(t1);
				t2 = omega * a_1[k+j+m/2];
				t2 = mod(t2);

				u1 = a_0[k+j];
				u2 = a_0[k+j+m/2];

				a_0[k+j] = u1 + t1;
				a_0[k+j] = mod(a_0[k+j]);
				a_1[k+j] = u2 + t2;
				a_1[k+j] = mod(a_1[k+j]);
				a_0[k+j+m/2] = u1 - t1;
				a_0[k+j+m/2] = mod(a_0[k+j+m/2]);
				a_1[k+j+m/2] = u2 - t2;
				a_1[k+j+m/2] = mod(a_1[k+j+m/2]);
			}
			omega = omega * primrt;
			omega = mod(omega);
		}
	}


	primrt = 5118; 	//mpz_set_str(primrt,"5118",10);
	omega = 1065;	//mpz_set_str(omega,"1065",10);
	for(j=0; j<M/2; j++)
	{
		t1 = omega * a_1[j];
		t1 = mod(t1);
		u1 = a_0[j];
		a_0[j] = u1 + t1;
		a_0[j] = mod(a_0[j]);
		a_1[j] = u1 - t1;
		a_1[j] = mod(a_1[j]);

		omega = omega * primrt;
		omega = mod(omega);
	}
}


void a_gen( int a_0[],  int a_1[])
{
	 int i, r;

	for(i=0;i<128;i++)
	{
		r = rand();
		a_0[i] = mod(r&0xffff);
		a_1[i] = mod((r>>16));

		//r = rand();
		//a_0[i] = mod(r);
		//r = rand();
		//a_1[i] = mod(r);
	}

	fwd_ntt(a_0, a_1);
}

int compare2(int a_0[128],int a_1[128],uint16_t large[M])
{
	int j;
	for (j=0; j<128; j++)
	{
		if ((large[2*j]!=a_0[j]) || (large[2*j+1]!=a_1[j]))
		{
			printf("j=%d",j);
			return 0;
			break;
		}
	}

	return 1;
}

void knuth_yao( int e_0[],  int e_1[])
{
	int r;
	int distance;
	int row, column;
	int sample_cntr;

	int bit, input;
	int index, sample, sample_msb, random;

	int integer_equivalent;
	int flag1;
	int ran;
	int sample_in_table;

	uint32_t rnd;
	int found;
	rnd=get_rand();


	for(sample_cntr=0; sample_cntr<128; sample_cntr++)
	{
		e_0[sample_cntr] = knuth_yao_single_number(&rnd,&sample_in_table);
		e_1[sample_cntr] = knuth_yao_single_number(&rnd,&sample_in_table);
		/*
		flag1=1;
		rnd = rand();
		index = rnd&0xff;
		sample = lut1[index]; //256 elements in lut1
		sample_msb = sample & 16;
		if(sample_msb==0)	  //lookup was successful
		{
			sample = sample & 0xf;
			if ((rnd>>8)&1) sample = (MODULUS - sample); //9th bit in rnd is the sign
			if(sample_cntr%2==1) e_1[sample_cntr/2] = sample;	// mpz_set_ui(e_1[SAMPLE_COUNTER/2], sample);
			if(sample_cntr%2==0) e_0[sample_cntr/2] = sample; //mpz_set_ui(e_0[SAMPLE_COUNTER/2], sample);
		}
		else
		{
			distance = sample & 7;
			rnd = rnd >> 8;
			index = rnd&0xff;
			sample = lut2[index]; //224 elements in lut2
			sample_msb = sample & 32;
			if(sample_msb==0)	// lookup was successful
			{
				sample = sample & 31;
				if ((rnd>>8)&1) sample = (MODULUS - sample); //9th bit in rnd is the sign
				if(sample_cntr%2==1) e_1[sample_cntr/2] = sample;	//mpz_set_ui(e_1[SAMPLE_COUNTER/2], sample);
				if(sample_cntr%2==0) e_0[sample_cntr/2] = sample; //mpz_set_ui(e_0[SAMPLE_COUNTER/2], sample);
			}

			//Real knuth-yao
			if(sample_msb!=0)
			{
				//rnd = rnd >> 8;
				distance = sample & 15;
				found=0;
				rnd=rnd>>7;
				for(column=13; ((found==0) && (column<109)); column++)
				{
					if ((column==32)||(column==64)||(column==96))
					{
						rnd=rand();
					}
					else
					{
						rnd = rnd>>1;
					}
					distance = distance*2 + (rnd&1);

					//distance = distance*2 + rand()&1;

					// Read probability-column 0 and count the number of non-zeros
					for(row=54; row>=0; row--)
					{
						distance = distance - pmat[row][column];
						if(distance<0)
						{
							if (rnd&1)
								sample = (MODULUS - row);
							else
								sample = row;
							if(sample_cntr%2==1) e_1[sample_cntr/2] = sample;	//mpz_set_ui(e_1[SAMPLE_COUNTER/2], sample);
							if(sample_cntr%2==0) e_0[sample_cntr/2] = sample;	//mpz_set_ui(e_0[SAMPLE_COUNTER/2], sample);
							found=1;

							break;
						}
					}
					//rnd = rnd >> 1;
				}
			}
		}*/

//		if (sample_cntr==2)
//			break;
	}
}
