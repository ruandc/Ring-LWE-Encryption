#include <stm32f4xx.h>
#include <misc.h>			 // I recommend you have a look at these in the ST firmware folder
#include <stm32f4xx_usart.h> // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src

#include <stm32f4xx_usart.h>
#include <stm32f4xx_rcc.h>
#include <lwe.h>
#include <test_asm.h>
#include "global.h"
#include "speed_test.h"
#include "term_io.h"
#include "stdlib.h"
#include "knuth_yao_asm.h"
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

/* This funcion initializes the USART1 peripheral
 * 
 * Arguments: baudrate --> the baudrate at which the USART is 
 * 						   supposed to operate
 */
/*
void init_USART1(uint32_t baudrate){
	

	GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6 (TX) and 7 (RX) are used
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// this activates the pullup resistors on the IO pins
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	
	USART_InitStruct.USART_BaudRate = baudrate;				// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(USART1, &USART_InitStruct);					// again all the properties are passed to the USART_Init function which takes care of all the bit setting
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt 
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		 // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff	

	// finally this enables the complete USART1 peripheral
	USART_Cmd(USART1, ENABLE);
}
*/
/* This function is used to transmit a string of characters via 
 * the USART specified in USARTx.
 * 
 * It takes two arguments: USARTx --> can be any of the USARTs e.g. USART1, USART2 etc.
 * 						   (volatile) char *s is the string you want to send
 * 
 * Note: The string has to be passed to the function as a pointer because
 * 		 the compiler doesn't know the 'string' data type. In standard
 * 		 C a string is just an array of characters
 * 
 * Note 2: At the moment it takes a volatile char because the received_string variable
 * 		   declared as volatile char --> otherwise the compiler will spit out warnings
 * */
/*

#define MAX_STRLEN 12 // this is the maximum string length of our string in characters
volatile char received_string[MAX_STRLEN+1]; // this will hold the recieved string

void USART_puts(USART_TypeDef* USARTx, volatile char *s){

	while(*s){
		// wait until data register is empty
		while( !(USARTx->SR & 0x00000040) ); 
		USART_SendData(USARTx, *s);
		*s++;
	}
}*/

void initAll ()
{
	//init_USART1(115200); // initialize USART1 @ 9600 baud
	comm_init();

	//RNG Peripheral enable
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}


/*
#ifdef NTT512
#else
uint16_t primrt_inv_omega_table[7] = {7680,3383,5756,1728,7584,6569,6601};
uint16_t primrt_omega_table_asm[8] = {7680,4298,6468,849,2138,3654,1714,5118};
#endif*/




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
