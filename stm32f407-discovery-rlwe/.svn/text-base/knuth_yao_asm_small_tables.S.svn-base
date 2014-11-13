
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

  .global knuth_yao_asm_small_tables
  .extern knuth_yao_asm_small_tables
  .type knuth_yao_asm_small_tables, %function
knuth_yao_asm_small_tables:
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

	mov r11,r0 //Save *return_vals to r11

	//r0=rnd
	//r1=str return val index
	//r2=lut1
	//r3=lut1_lookup, lut2_lookup, sample,row (return values)
	//r4=tmp: sample_msb, index, column
	//r5=MODULUS
	//r6=distance (only used from lut2_lookup_continue)
	//r7=temp (clz)
	//r8=pmat_cols_small_low
	//r9=pmat_cols_small_high/lut2
	//r10=low,high
	//r11=*return_vals
	//r12=(M-1)
	//r14=result from every second (odd numbered) calculation

	ldr r2,=lut1
	ldr r5,=MODULUS
	#ifdef USE_SMALL_TABLES
		ldr r8,=pmat_cols_small_low2
	#else
		ldr r8,=pmat_cols_small_low
	#endif
	//ldr r9,=pmat_cols_small_high
	ldr r9,=lut2
	mov r1,#-1
	GET_RAND; //Get the initial random number
	//ldr	r0, =#RNG_ADDR
	//ldr	r0, [r0, #8]
	ldr r12,=#(M-1)

knuth_yao_single:

	uxtb r4,r0 //r4 = index
	lsr r0,#8 //(*rnd)=(*rnd)>>8;

	GET_RAND_IF_NECESSARY(lookup_lut1)
	//ldr r2, =lut1
	ldrb r3, [r2,r4]; //sample //sample (r3) = lut1[index]; //256 elements in lut1

	lsrs r4,r3,#5 //sample_msb = sample & 16 = (2^4) = fifth bit
	bcs lut1_no_success

lut1_success_continue:
	//lut1 lookup SUCCESS

	//NB: This was part of the original LWE1.c, but seems to be unneccesary:
	//and r3,r3,#31;//sample = sample & 0xf;

	lsrs r0,#1 //rnd (r0) = rnd >> 1
	bcc cleanup //rnd_rem-- in cleanup

	sub r3,r5,r3 ////sample (r3) = (MODULUS - sample);
	b cleanup //rnd_rem-- in cleanup

lut1_no_success:
	//lut1 lookup FAIL: Now try lut2 lookup

	//Check if enough bits exist inside rnd
	clz r4,r0

	cmp r4,#NEW_RND_MID//cmp r4,#27
	ble lut2_lookup_continue
	GET_RAND

lut2_lookup_continue:
	and r6,r3,#KN_DISTANCE1_MASK; //distance(r6) = sample(r3) & 7;
	and r4,r0,#0x1f //r4 = (*rnd)&0x1f
	add r4,r4,r6,lsl #5 //index (r4) = [(*rnd)&0x1f] (r4) + 32*distance(r6);

	lsr r0,#5; //(*rnd) = (*rnd)>>5;

	GET_RAND_IF_NECESSARY(lut2_lookup_continue2)

	ldrb r3, [r9,r4]; //sample (r3) = lut2[index]; //224 elements in lut2

	lsrs r4,r3,#6 //The sixth bit (2^5) represents the sample_msb
	bcs lut2_no_success_start_real_kn

	//lut2 SUCCESS!
	and r3,#31 //sample (r3) = sample & 31;

	lsrs r0,#1
	bcc cleanup //rnd_rem-- in cleanup

	sub r3,r5,r3 //if ((*rnd)&1) sample (r3) = (MODULUS - sample)
	b cleanup //rnd_rem-- in cleanup

lut2_no_success_start_real_kn:
	//Now we perform bit scanning with knuth-yao
	and r6,r3,#15; //distance (r6) = sample(r3) & 15;

	//for(column=0; column<HAMMING_TABLE_SIZE; column++) {
	mov r4,#0 //column(r4)=0
real_kn_low_loop:
	/*******Start of (big) inner loop*******/

	/*distance = distance*2 + ((*rnd)&1);*/
	and r10,r0,#1 			//r10=((*rnd)&1);
	lsrs r0,#1 				//(*rnd)=(*rnd)>>1;
	add r6,r10,r6,lsl #1	//distance (r6) = ((*rnd)&1) + distance*2

	GET_RAND_IF_NECESSARY(real_kn_low_loop_continue)

	ldr r10,[r8, r4, lsl #2]; //low(r10)=pmat_cols_small_low[column];

	mov r3,#LOW_MSB_PLUS_ONE;
	BIT_SCANNING(real_kn_low_loop1,finished_exit)

	add r4,#1
	cmp r4,#HAMMING_TABLE_SIZE
	blt real_kn_low_loop
	//*******End of (big) inner loop*******

	//At this point we still haven't found the gaussian sample.
	//We now have to use both the high and low values to perform the sampling.
	#ifdef USE_SMALL_TABLES
		ldr r9,=pmat_cols_small_high3
	#else
		ldr r9,=pmat_cols_small_high
	#endif
real_kn_high_loop:

	//for(column=HAMMING_TABLE_SIZE; (column<(109-13)); column++)	{
	and r10,r0,#1 			//r10=((*rnd)&1);
	lsr r0,#1 				//(*rnd)(r0)=(*rnd)>>1;
	add r6,r10,r6,lsl #1	//distance (r6) = ((*rnd)&1) + distance*2

	GET_RAND_IF_NECESSARY(real_kn_high_loop_continue)

	//***********Start of outer high loop**************

	#ifdef USE_SMALL_TABLES
		//If we use pmat_cols_small_high3 here then we need to subtract HAMMING_TABLE_SIZE
		//from r4 to read the correct entry in pmat_cols_small_high3
		ldr	r3,=#HAMMING_TABLE_SIZE
		sub r3,r4,r3
		ldr r10,[r9, r3, lsl #2]; //high(r10)=pmat_cols_small_high[column];
	#else
		ldr r10,[r9, r4, lsl #2]; //high(r10)=pmat_cols_small_high[column];
	#endif


	cmp r10,#0
	beq low_loop_cont

	//Skip BIT_SCANNING here because r10==0
	mov r3,#PMAT_MAX_ROW_ONE;
	BIT_SCANNING(real_kn_high_loop1,high_low_finished_exit)
low_loop_cont:
	mov r3,#LOW_MSB_PLUS_ONE //e3 might not get set to LOW_MSB_PLUS_ONE if pmat_cols_small_high=0
	ldr r10,[r8, r4, lsl #2]; //low(r10)=pmat_cols_small_low[column];
	BIT_SCANNING(low_loop,high_low_finished_exit)

	add r4,#1 //column(r4) ++
	b real_kn_high_loop //No tests are necessary: the loop should end in here
	//***********End of outer high loop**************

high_low_finished_exit:
	ldr r9,=lut2 //Store @lut2 inside r9

finished_exit:
	//FINISHED: Exit!
	//NB: r3 is used as an input here!
	lsrs r0,#1 //r0 = random number
	bcc cleanup //rnd_rem-- in cleanup

	sub r3,r5,r3 //sample (r3) = (MODULUS - sample); //9th bit in rnd is the sign

cleanup:
	clz r6,r0
	//cmp r6,#24
	cmp r6,#NEW_RND_LARGE
	/*ble knuth_yao_asm_done //if ((*rnd_rem)<8)
	GET_RAND*/
	GET_RAND2(knuth_yao_asm_done)

knuth_yao_asm_done:

	add r14,r3,r14,lsl #16 //We combine two results into a single word

	lsrs r7,r1,#1
	it cc
	strcc r14,[r11,r1,lsl #1]; //Write combined_results (r14) to [r11]

	add r1,#1
	cmp r1,r12 //cmp r1,#(M-1) //Check if r1==256
	bne knuth_yao_single //Jump to knuth_yao_single

	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}
