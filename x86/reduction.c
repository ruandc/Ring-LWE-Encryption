#include <stdint.h>
#include <stdio.h>

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

int mul_inv(int a, int b)
{
	int b0 = b, t, q;
	int x0 = 0, x1 = 1;
	if (b == 1) return 1;
	while (a > 1) {
		q = a / b;
		t = b, b = a % b, a = t;
		t = x0, x0 = x1 - q * x0, x1 = t;
	}
	if (x1 < 0) x1 += b0;
	return x1;
}

void unit_test_reduction_longa()
{
	int i;
	int res;

	printf("mod_longa: ");
	res = 1;
	uint32_t k_inv=mul_inv(3, 12289);
	for (i=-12289*8; i<=12289*8; i++)
	{
		int32_t in=i;
		int32_t scaled_in=in*k_inv;

		uint32_t output = mod_longa(scaled_in);
		output = mod(output); //mod_longa doesn't provide a "true" mod operation. Put it in the same range as expected value

		uint32_t expected = mod(in);

		if (output!=expected)
		{
			printf("in=%x output=%x expected=%x\n\r",in,output,expected);
			res=0;
			break;
		}
	}
	if (res==0)
		printf("BAD!\n");
	else
		printf("OK!\n");

	printf("mod_longa_2x: ");
	res = 1;
	for (i=-12289*8; i<=12289*8; i++)
	{
		uint32_t in=i;
		int32_t scaled_in=(uint32_t)in*k_inv;

		uint32_t output=mod_longa_2x(scaled_in);
		uint32_t output_scaled = mod(output*k_inv);

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
