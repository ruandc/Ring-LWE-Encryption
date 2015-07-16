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
	/**** while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
	 8000276:	2001      	movs	r0, #1
 	 8000278:	f00a f8c6 	bl	800a408 <RNG_GetFlagStatus>
 	 800027c:	4603      	mov	r3, r0
 	 800027e:	2b00      	cmp	r3, #0
 	 8000280:	d0f9      	beq.n	8000276 <TM_RNG_Get+0x6>
	 *
	 *
	 **** return RNG->DR;
	 * ldr	r3, [pc, #8]	; (800028c <TM_RNG_Get+0x1c>)
 	 8000284:	689b      	ldr	r3, [r3, #8]
 	 800028c:	50060800 	.word	0x50060800

	 //equivalent:
 	 ldr r0,=0x50060800
 	 ldr r0, [r0, #8]
	 */

	//#define PERIPH_BASE           ((uint32_t)0x40000000) /*!< Peripheral base address in the alias region                                */
	//#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000)
	//#define RNG_BASE              (AHB2PERIPH_BASE + 0x60800)
	//RNG_BASE = 0x50060800

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

	int i,j;
	for (i=0; i<10; i++){
		xprintf("rand:%x\n",get_rand());
	}

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

/*
int main(void) {

	initAll();

	USART_puts(USART1, "Init complete! Hello World!\r\n"); // just send a message to indicate that it works


  	uint32_t large1[M], large2[M];
	int i,fail;
	xprintf("knuth_yao_asm:");
	fail=0;
	for (i=0x0; i<0xFF; i++)
	{
		srand(i);
		knuth_yao_asm(large1);

		srand(i);
		knuth_yao2(large2);

		if (!compare_large_simd(large1,large2))
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
	{
		xprintf("FAIL i=%x\n",i);
	}
	else
	{
		xprintf("OK\n");
	}

  int a;
  while (1){
	  a = TM_RNG_Get();
  }
}*/

/*
// this is the interrupt request handler (IRQ) for ALL USART1 interrupts
void USART1_IRQHandler(void){
	
	// check if the USART1 receive interrupt flag was set
	if( USART_GetITStatus(USART1, USART_IT_RXNE) ){
		
		static uint8_t cnt = 0; // this counter is used to determine the string length
		char t = USART1->DR; // the character from the USART1 data register is saved in t
		
		// check if the received character is not the LF character (used to determine end of string)
		// or the if the maximum string length has been been reached

		if( (t != '\n') && (cnt < MAX_STRLEN) ){ 
			received_string[cnt] = t;
			cnt++;
		}
		else{ // otherwise reset the character counter and print the received string
			cnt = 0;
			USART_puts(USART1, received_string);
		}
	}
}
*/
