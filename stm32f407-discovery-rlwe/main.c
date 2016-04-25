#include <stm32f4xx.h>
#include <misc.h>			 // I recommend you have a look at these in the ST firmware folder
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src
#include <stm32f4xx_usart.h>
#include <stm32f4xx_rcc.h>
#include <lwe.h>
#include "global.h"
#include "lwe_arm.h"
#include "speed_test.h"
#include "term_io.h"
#include "stdlib.h"
#include "unit_test.h"

void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

uint32_t TM_RNG_Get(void) {
	//Wait until one RNG number is ready
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);

	//Get a 32bit Random number
	//return RNG_GetRandomNumber();
	return RNG->DR;
}

void initAll ()
{
	comm_init();

	//RNG Peripheral enable
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}

void print_array(uint32_t a[])
{
	for (int i=0; i<M; i++)
	{
		xprintf("%d ",a[i]);
	}
}

int main (void)
{
	// Initialize everything
	initAll();

	xprintf("Startup!");

#ifdef GENERATE_ASCII_RANDOM_BITS

	uint32_t i,j;
	uint32_t n;
	for (i=0; i<NUMBER_OF_RANDOM_WORDS; i++)
	{
		n = TM_RNG_Get();
		//while (n) {
		for (j=0; j<32; j++)
		{
		    if (n & 1)
		        xprintf("1");
		    else
		        xprintf("0");

		    n >>= 1;
		}
	}

#endif

#ifdef GENERATE_BINARY_RANDOM_BITS
	uint32_t i=0;
	uint32_t n;
	for (i=0; i<NUMBER_OF_RANDOM_WORDS; i++)
	{
		//xprintf(".");
		n = TM_RNG_Get();
		xprintf("%c%c%c%c",(n&0xff),((n>>8)&0xff),((n>>16)&0xff),((n>>24)&0xff));
	}
#endif

#ifdef PERFORM_UNIT_TESTS
	perform_unit_tests();
#endif

#ifdef PERFORM_SPEED_TESTS
	speed_test();
#endif

	xputs("[Done]\n\n");

	while (1)
	{
	}

	return 0;
}
