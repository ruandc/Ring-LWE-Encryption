
#include "global.h"

#ifdef USE_FAKE_GET_RAND
	#define GET_RAND\
		ldr	r0, =g_fake_rand;\
		ldr r0, [r0];\
		orr r0, #0x80000000;
#else
	#ifdef USE_TRNG
		#define GET_RAND\
			ldr	r0, =#RNG_ADDR;\
		 	ldr	r0, [r0, #8];\
		 	orr r0, #0x80000000;
	#else
		#define GET_RAND\
			push {r1,r2,r3,lr};\
			bl	rand;\
			orr r0, #0x80000000;\
			pop {r1,r2,r3,lr};
	#endif
#endif

#define GET_RAND_IF_NECESSARY(id)\
	/*cbnz r0,id;*/;\
	cmp r0,#NEW_RND_BOTTOM;\
	bgt id;\
	GET_RAND;\
	id:

//The GET_RAND2 trick is sometimes faster
#ifdef USE_TRNG
	#define GET_RAND2(id)\
		ble id;\
		ittt le;\
		ldrle	r0, =#RNG_ADDR;\
	 	ldrle	r0, [r0, #8];\
	 	orrle r0,#0x80000000;
#else
	#define GET_RAND2(id)\
		ble id;\
		push {r1,r2,r3,lr};\
		bl	rand;\
		orr r0,#0x80000000;\
		pop {r1,r2,r3,lr};
#endif

#define BIT_SCANNING(id,jump_label)\
	id:;\
	clz r7,r10;\
	add r7,#1;\
	lsl r10,r7;\
	sub r3,r7;\
	subs r6,#1;\
	bmi jump_label;\
	cmp r10,#0;\
	bne id;

  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.section .text

  .balign  2
  .thumb
  .thumb_func
  .code 16

  .global this_test
  .extern this_test
  .type this_test, %function
knuth_yao_asm_shuffle:
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

	mov r11,r0 //Save *return_vals to r11

	add 
knuth_yao_asm_done_no_lut:

	//add r14,r3,r14,lsl #16 //We combine two results into a single word

	//Only write the result to memory if r1 is even
	//lsrs r7,r1,#1
	//it cc
	//strcc r14,[r11,r1,lsl #1]; //Write combined_results (r14) to [r11]

	//add r1,#1
	strh r3,[r11,r12,lsl #1]
	sub r12,#1
	cmp r1,r12 //cmp r1,#(M-1) //Check if r1==256
	bne knuth_yao_single //Jump to knuth_yao_single

	add r0,r12,#1 //return counter2
	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}
