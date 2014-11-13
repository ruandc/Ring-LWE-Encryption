
  .global knuth_yao_single_number_asm
  .extern knuth_yao_single_number_asm
  .type knuth_yao_single_number_asm, %function

knuth_yao_single_number_asm:
	stmdb	sp!, {r0, r4, r5, r6, r7, r8, r9, r10, lr}

	//Dereference the pointers
	ldr r0, [r0]

	//r0=rnd
	//r2=lut1
	//r3=sample,row (return values)
	//r4=tmp: sample_msb, index, column
	//r5=MODULUS
	//r6=distance
	//r7=lut2, temp (clz)
	//r8=pmat_cols_small_low
	//r9=pmat_cols_small_high
	//r10=low,high

	uxtb r4,r0 //r4 = index
	lsr r0,#8 //(*rnd)=(*rnd)>>8;
	cbnz r0,initial_skip
	GET_RAND

initial_skip:

	ldr r2, =lut1
	ldrb r3, [r2,r4]; //sample (r3) = lut1[index]; //256 elements in lut1

	lsrs r4,r3,#5 //sample_msb = sample & 16 = (2^4) = fifth bit
	bcs lut1_no_success

lut1_success_continue:
	//lut1 lookup SUCCESS
	and r3,r3,#31;//sample = sample & 0xf;
	lsrs r0,#1 //rnd (r0) = rnd >> 1
	bcc cleanup //rnd_rem-- in cleanup

	ldr r5,=MODULUS
	sub r3,r5,r3 ////sample (r3) = (MODULUS - sample);
	b cleanup //rnd_rem-- in cleanup

lut1_no_success:
	//lut1 lookup FAIL: Now try lut2 lookup

	//Check if enough bits exist inside rnd
	//bpl lut2_lookup_continue
	clz r7,r0
	cmp r7,#(27)
	ble lut2_lookup_continue
	GET_RAND

lut2_lookup_continue:
	and r6,r3,#KN_DISTANCE1_MASK; //distance(r6) = sample(r3) & 7;
	and r4,r0,#0x1f //r4 = (*rnd)&0x1f
	add r4,r4,r6,lsl #5 //index (r4) = [(*rnd)&0x1f] (r4) + 32*distance(r6);

	lsr r0,#5; //(*rnd) = (*rnd)>>5;
	cbnz r0,lut2_lookup_continue2 //if ((*rnd)==0)
	GET_RAND

lut2_lookup_continue2:
	ldr r7, =lut2
	ldrb r3, [r7,r4]; //sample (r3) = lut2[index]; //224 elements in lut2

	lsrs r4,r3,#6 //The sixth bit (2^5) represents the sample_msb
	bcs lut2_no_success_start_real_kn

	//lut2 SUCCESS!
	and r3,#31 //sample (r3) = sample & 31;

	lsrs r0,#1
	bcc cleanup //rnd_rem-- in cleanup

	ldr r5,=MODULUS
	sub r3,r5,r3 //if ((*rnd)&1) sample (r3) = (MODULUS - sample)
	b cleanup //rnd_rem-- in cleanup

lut2_no_success_start_real_kn:
	//Now we perform bit scanning with knuth-yao
	and r6,r3,#KN_DISTANCE2_MASK; //distance (r6) = sample(r3) & 15;

	ldr r8,=pmat_cols_small_low

	//for(column=0; column<HAMMING_TABLE_SIZE; column++) {
	mov r4,#0 //r4=column
real_kn_low_loop:
	/*******Start of (big) inner loop*******/

	//distance = distance*2 + ((*rnd)&1);
	and r10,r0,#1 //r10=((*rnd)&1);
	lsrs r0,#1 //(*rnd)=(*rnd)>>1;
	add r6,r10,r6,lsl #1//distance (r6) = ((*rnd)&1) + distance*2

	cbnz r0,real_kn_low_loop_continue
	GET_RAND
real_kn_low_loop_continue:

	ldr r10,[r8, r4, lsl #2]; //low=pmat_cols_small_low[column];
	clz r7,r10
	lsl r10,r7

	//for(row=(54-32); row>=0; row--)
	mov r3,#LOW_MSB //r3=row
	sub r3,r7 //r7=free

real_kn_low_loop_inner_continue:
	//******Start of (small) inner loop ******

	lsls r10,#1 //low(r10)=low<<1;
	bcc real_kn_low_loop_inner_continue2
	subs r6,#1 //distance (r6) = distance - (low>>31); //subtract the most significant bit
	bmi finished_exit //if (distance(r6)==-1) {

real_kn_low_loop_inner_continue2:
	subs r3,#1 //row=row-1
	bpl real_kn_low_loop_inner_continue //row>=0
	//******End of (small) inner loop ******

	add r4,#1
	cmp r4,#HAMMING_TABLE_SIZE
	bne real_kn_low_loop
	//*******End of (big) inner loop*******

	//At this point we still haven't found the gaussian sample.
	//We now have to use both the high and low values to perform the sampling.
real_kn_high_loop:

	//for(column=HAMMING_TABLE_SIZE; (column<(109-13)); column++)	{
	and r10,r0,#1 //r10=((*rnd)&1);
	lsr r0,#1 //(*rnd)(r0)=(*rnd)>>1;
	add r6,r10,r6,lsl #1//distance (r6) = ((*rnd)&1) + distance*2

	ldr r9,=pmat_cols_small_high

	cbnz r0,real_kn_high_loop_continue
	GET_RAND
real_kn_high_loop_continue:
	//***********Start of outer high loop**************

	ldr r10,[r9, r4, lsl #2]; //high(r10)=pmat_cols_small_high[column];
	clz r7,r10
	mov r3,#54 //r3=row
	sub r3,r7 //r7=free

	cmp r3,#LOW_MSB
	beq low_loop_init //if r10 is empty (r3=LOW_MSB) then there is no need to process 'high'
	lsl r10,r7 //Remove leading zeros from high (r10)

	//for(row=54; row>(54-32); row--)
high_loop:
	//***********Start of high loop**************
	lsls r10,#1 //low(r10)=low<<1;
	bcc high_loop_continue
	subs r6,#1 //distance (r6) = distance - (high>>31); //subtract the most significant bit
	bmi finished_exit //if (distance(r6)==-1) {

high_loop_continue:
	sub r3,#1 //row--
	cmp r3,#LOW_MSB
	bgt high_loop
	//***********End of high loop**************

low_loop_init:
	ldr r10,[r8, r4, lsl #2]; //low(r10)=pmat_cols_small_low[column];

	//for(row=(54-32); row>=0; row--)
low_loop:
	//***********Start of low loop**************

	lsls r10,#1 //low(r10)=low<<1;
	bcc low_loop_continue
	subs r6,#1 //distance (r6) = distance - (low>>31); //subtract the most significant bit
	bmi finished_exit //if (distance(r6)==-1) {

low_loop_continue:
	subs r3,#1 //row--
	bpl low_loop
	//***********End of low loop**************

	add r4,#1 //column(r4) ++
	b real_kn_high_loop //No tests are necessary: the loop should end in here
	//***********End of outer high loop**************

finished_exit:
	//FINISHED: Exit!
	//NB: r3 is used as an input here!
	lsrs r0,#1 //r0 = random number
	bcc cleanup //rnd_rem-- in cleanup

	ldr r5,=MODULUS
	sub r3,r5,r3 //sample (r3) = (MODULUS - sample); //9th bit in rnd is the sign

cleanup:
	clz r2,r0
	cmp r2,#(24)
	ble final_cleanup //if ((*rnd_rem)<8)
	GET_RAND

final_cleanup:
	//Rereference the pointers
	pop {r4};
	str r0, [r4]

	mov r0,r3; //Not sure why this is here. w/o this the result is wrong
	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, pc}

  .global knuth_yao_single_number_asm
  .extern knuth_yao_single_number_asm
  .type knuth_yao_single_number_asm, %function

knuth_yao_single_number_asm:
	stmdb	sp!, {r0, r4, r5, r6, r7, r8, r9, r10, lr}

	//Dereference the pointers
	ldr r0, [r0]

	//r0=rnd
	//r2=lut1
	//r3=sample,row (return values)
	//r4=tmp: sample_msb, index, column
	//r5=MODULUS
	//r6=distance
	//r7=lut2, temp (clz)
	//r8=pmat_cols_small_low
	//r9=pmat_cols_small_high
	//r10=low,high

	uxtb r4,r0 //r4 = index
	lsr r0,#8 //(*rnd)=(*rnd)>>8;
	cbnz r0,initial_skip
	GET_RAND

initial_skip:

	ldr r2, =lut1
	ldrb r3, [r2,r4]; //sample (r3) = lut1[index]; //256 elements in lut1

	lsrs r4,r3,#5 //sample_msb = sample & 16 = (2^4) = fifth bit
	bcs lut1_no_success

lut1_success_continue:
	//lut1 lookup SUCCESS
	and r3,r3,#31;//sample = sample & 0xf;
	lsrs r0,#1 //rnd (r0) = rnd >> 1
	bcc cleanup //rnd_rem-- in cleanup

	ldr r5,=MODULUS
	sub r3,r5,r3 ////sample (r3) = (MODULUS - sample);
	b cleanup //rnd_rem-- in cleanup

lut1_no_success:
	//lut1 lookup FAIL: Now try lut2 lookup

	//Check if enough bits exist inside rnd
	//bpl lut2_lookup_continue
	clz r7,r0
	cmp r7,#(27)
	ble lut2_lookup_continue
	GET_RAND

lut2_lookup_continue:
	and r6,r3,#7; //distance(r6) = sample(r3) & 7;
	and r4,r0,#0x1f //r4 = (*rnd)&0x1f
	add r4,r4,r6,lsl #5 //index (r4) = [(*rnd)&0x1f] (r4) + 32*distance(r6);

	lsr r0,#5; //(*rnd) = (*rnd)>>5;
	cbnz r0,lut2_lookup_continue2 //if ((*rnd)==0)
	GET_RAND

lut2_lookup_continue2:
	ldr r7, =lut2
	ldrb r3, [r7,r4]; //sample (r3) = lut2[index]; //224 elements in lut2

	lsrs r4,r3,#6 //The sixth bit (2^5) represents the sample_msb
	bcs lut2_no_success_start_real_kn

	//lut2 SUCCESS!
	and r3,#31 //sample (r3) = sample & 31;

	lsrs r0,#1
	bcc cleanup //rnd_rem-- in cleanup

	ldr r5,=MODULUS
	sub r3,r5,r3 //if ((*rnd)&1) sample (r3) = (MODULUS - sample)
	b cleanup //rnd_rem-- in cleanup

lut2_no_success_start_real_kn:
	//Now we perform bit scanning with knuth-yao
	and r6,r3,#15; //distance (r6) = sample(r3) & 15;

	ldr r8,=pmat_cols_small_low

	//for(column=0; column<HAMMING_TABLE_SIZE; column++) {
	mov r4,#0 //r4=column
real_kn_low_loop:
	/*******Start of (big) inner loop*******/

	//distance = distance*2 + ((*rnd)&1);
	and r10,r0,#1 //r10=((*rnd)&1);
	lsrs r0,#1 //(*rnd)=(*rnd)>>1;
	add r6,r10,r6,lsl #1//distance (r6) = ((*rnd)&1) + distance*2

	cbnz r0,real_kn_low_loop_continue
	GET_RAND
real_kn_low_loop_continue:

	ldr r10,[r8, r4, lsl #2]; //low=pmat_cols_small_low[column];
	clz r7,r10
	lsl r10,r7

	//for(row=(54-32); row>=0; row--)
	mov r3,#LOW_MSB //r3=row
	sub r3,r7 //r7=free

real_kn_low_loop_inner_continue:
	//******Start of (small) inner loop ******

	lsls r10,#1 //low(r10)=low<<1;
	bcc real_kn_low_loop_inner_continue2
	subs r6,#1 //distance (r6) = distance - (low>>31); //subtract the most significant bit
	bmi finished_exit //if (distance(r6)==-1) {

real_kn_low_loop_inner_continue2:
	subs r3,#1 //row=row-1
	bpl real_kn_low_loop_inner_continue //row>=0
	//******End of (small) inner loop ******

	add r4,#1
	cmp r4,#HAMMING_TABLE_SIZE
	bne real_kn_low_loop
	//*******End of (big) inner loop*******

	//At this point we still haven't found the gaussian sample.
	//We now have to use both the high and low values to perform the sampling.
real_kn_high_loop:

	//for(column=HAMMING_TABLE_SIZE; (column<(109-13)); column++)	{
	and r10,r0,#1 //r10=((*rnd)&1);
	lsr r0,#1 //(*rnd)(r0)=(*rnd)>>1;
	add r6,r10,r6,lsl #1//distance (r6) = ((*rnd)&1) + distance*2

	ldr r9,=pmat_cols_small_high

	cbnz r0,real_kn_high_loop_continue
	GET_RAND
real_kn_high_loop_continue:
	//***********Start of outer high loop**************

	ldr r10,[r9, r4, lsl #2]; //high(r10)=pmat_cols_small_high[column];
	clz r7,r10
	mov r3,#54 //r3=row
	sub r3,r7 //r7=free

	cmp r3,#LOW_MSB
	beq low_loop_init //if r10 is empty (r3=LOW_MSB) then there is no need to process 'high'
	lsl r10,r7 //Remove leading zeros from high (r10)

	//for(row=54; row>(54-32); row--)
high_loop:
	//***********Start of high loop**************
	lsls r10,#1 //low(r10)=low<<1;
	bcc high_loop_continue
	subs r6,#1 //distance (r6) = distance - (high>>31); //subtract the most significant bit
	bmi finished_exit //if (distance(r6)==-1) {

high_loop_continue:
	sub r3,#1 //row--
	cmp r3,#LOW_MSB
	bgt high_loop
	//***********End of high loop**************

low_loop_init:
	ldr r10,[r8, r4, lsl #2]; //low(r10)=pmat_cols_small_low[column];

	//for(row=(54-32); row>=0; row--)
low_loop:
	//***********Start of low loop**************

	lsls r10,#1 //low(r10)=low<<1;
	bcc low_loop_continue
	subs r6,#1 //distance (r6) = distance - (low>>31); //subtract the most significant bit
	bmi finished_exit //if (distance(r6)==-1) {

low_loop_continue:
	subs r3,#1 //row--
	bpl low_loop
	//***********End of low loop**************

	add r4,#1 //column(r4) ++
	b real_kn_high_loop //No tests are necessary: the loop should end in here
	//***********End of outer high loop**************

finished_exit:
	//FINISHED: Exit!
	//NB: r3 is used as an input here!
	lsrs r0,#1 //r0 = random number
	bcc cleanup //rnd_rem-- in cleanup

	ldr r5,=MODULUS
	sub r3,r5,r3 //sample (r3) = (MODULUS - sample); //9th bit in rnd is the sign

cleanup:
	clz r2,r0
	cmp r2,#(24)
	ble final_cleanup //if ((*rnd_rem)<8)
	GET_RAND

final_cleanup:
	//Rereference the pointers
	pop {r4};
	str r0, [r4]

	mov r0,r3; //Not sure why this is here. w/o this the result is wrong
	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, pc}





	/*********UMOD_ASM*********
	* Input: r0
	* Output: A
	*/
	#define UMOD_ASM(A)\
		 mov r1,r0;\
		 ldr r3,=#UMOD_CONSTANT;\
		 smull	r0, r3, r3, r1;\
		 lsrs	r2, r3, #11;\
		 lsls	r3, r2, #4;\
		 subs	r3, r3, r2;\
		 add	r3, r2, r3, lsl #9;\
		 subs	A, r1, r3;

	/*********UMOD_ASM2*********
	* Input: A
	* Output: A
	* MODULUS: E
	* Temp:B,C,D
	*/
	//ldr D,=#UMOD_CONSTANT;
	#define UMOD_ASM2(A,B,C,D,E)\
		 mov B,A;\
		 smull	A, D, E, B;\
		 lsrs	C, D, #11;\
		 lsls	D, C, #4;\
		 subs	D, D, C;\
		 add	D, C, D, lsl #9;\
		 subs	A, B, D;


		.global umod_asm
		.extern umod_asm
		.type umod_asm, %function
	umod_asm:
		 //r1=input?
		 //r3=magic_number?
		 /*
		 mov r1,r0
		 ldr r3,=#UMOD_CONSTANT
		 smull	r0, r3, r3, r1
		 lsrs	r2, r3, #11
		 lsls	r3, r2, #4
		 subs	r3, r3, r2
		 add	r3, r2, r3, lsl #9
		 subs	r0, r1, r3*/
		 UMOD_ASM(r0)
		 mov pc,lr



			.global smod_asm
			.extern smod_asm
			.type smod_asm, %function
		smod_asm:
			//stmdb	sp!, {lr}

			ldr 	r3,	=#UMOD_CONSTANT

			mov 	r1,	r0//unnecessary?
			smull	r0, r3, r3, r1
			asrs	r2, r3, #11
			//asrs	r3, r1, #31
			//subs	r2, r2, r3
			subs	r2, r2, r1, asr #31
			//mov		r3, r2
			lsls	r3, r2, #4
			subs	r3, r3, r2
			//lsls	r3, r3, #9
			add		r3, r2, r3, lsl #9
			subs	r0, r1, r3

			//ldmia.w	sp!, {pc}
			mov pc,lr

			.global mod_asm
			.extern mod_asm
			.type mod_asm, %function
		mod_asm:
			cmp r0,	#0
			blt mod_asm_neg
			UMOD_ASM(r0)
			//b mod_asm_exit
			mov pc,lr
		mod_asm_neg:
			//quotient = a/MODULUS;
			ldr r1,=#MODULUS
			sdiv r2,r0,r1 //quotient(r2) = a(r0)/MODULUS(r1);

			//remainder = (1-quotient)*MODULUS + a;
			mov r3,#1
			sub r3,r3,r2
			mla r0,r3,r1,r0 //muls r3,r3,r1; add r0,r3,r0

		mod_asm_exit:
			mov pc,lr

			.global mod_asm2
			.extern mod_asm2
			.type mod_asm2, %function
		mod_asm2:

			//SMOD:
			ldr 	r3,	=#UMOD_CONSTANT;
			mov 	r1,	r0;
			smull	r0, r3, r3, r1;
			asrs	r2, r3, #11;
			subs	r2, r2, r1, asr #31;
			lsls	r3, r2, #4;
			subs	r3, r3, r2;
			add		r3, r2, r3, lsl #9;
			subs	r0, r1, r3
			//r1=input, r0=smod(r1)

			cmp r1,	#0
			bge mod_asm2_exit

			ldr r1,=#MODULUS
			add r0,r0,r1

		mod_asm2_exit:
			mov pc,lr

			.global mod_asm3
			.extern mod_asm3
			.type mod_asm3, %function
		mod_asm3:
			cmp r0,	#0
			blt mod_asm_neg3
			UMOD_ASM(r0)
			//b mod_asm_exit
			mov pc,lr
		mod_asm_neg3:
			//quotient = a/MODULUS;

			//Calculate: sdiv r2,r0,r1 //quotient(r2) = a(r0)/MODULUS(r1);
			ldr	r2, =#UMOD_CONSTANT
			smull	r1, r2, r2, r0
			asrs	r2, r2, #11
			asrs	r3, r0, #31
			subs	r2, r2, r3

			ldr r1,=#MODULUS

			//remainder = (1-quotient)*MODULUS + a;
			mov r3,#1
			sub r3,r3,r2
			mla r0,r3,r1,r0 //muls r3,r3,r1; add r0,r3,r0
			mov pc,lr

			.global mod_asm4
			.extern mod_asm4
			.type mod_asm4, %function
		mod_asm4:
			cmp r0,	#0
			blt mod_asm_neg4
			UMOD_ASM(r0)
			//b mod_asm_exit
			mov pc,lr
		mod_asm_neg4:
			//num32=umod_asm(-num16);
			//num32=7681-num32;

			//mov r2,#0
			//sub r0,r2,r0
			rsb r0,r0,#0
			UMOD_ASM(r0)
			ldr r1,=#MODULUS
			sub r0,r1,r0
			mov pc,lr


#define SMOD_DIV_ASM2(IN,MODREG,OUT,TMP)\
	cmp IN,	#0;\
	itte lt;\
	rsblt IN,IN,#0;\
	movlt TMP,MODREG;\
	movge TMP,#0;\
	udiv OUT,IN,MODREG;\
	mls OUT,OUT,MODREG,IN;\
	sub OUT,TMP,OUT

	.global smod_div_asm2
	.extern smod_div_asm2
	.type smod_div_asm2, %function
smod_div_asm2:
	ldr r1,=#MODULUS;
	SMOD_DIV_ASM2(r0,r1,r2,r3);
	mov r0,r2
	mov pc,lr

.global mod_asm5
	.extern mod_asm5
	.type mod_asm5, %function
mod_asm5:

	ldr r1,=#MODULUS;
	//SMOD_DIV_ASM(r0,r1,r2);
/*
	cmp r0,	#0
	it lt
	rsblt r0,r0,#0
	udiv r2,r0,r1
	mls r2,r2,r1,r0
	it lt
	sublt r2,r1,r2
	mov r0,r2
	*/
	cmp r0,	#0
	itte lt
	rsblt r0,r0,#0
	movlt r3,r1
	movge r3,#0
	udiv r2,r0,r1
	mls r2,r2,r1,r0
	sub r0,r3,r2
	mov pc,lr
/*
	cmp IN,	#0;\
	itte lt;\
	rsblt IN,IN,#0;\
	movlt TMP,MODREG;\
	movge TMP,#0;\
	udiv OUT,IN,MODREG;\
	mls OUT,OUT,MODREG,IN;\
	sub OUT,TMP,OUT
	*/
/*
	b mod_asm_neg5_exit
mod_asm_neg5:
	//num32=umod_asm(-num16);
	//num32=7681-num32;
	rsb r0,r0,#0
	sdiv r2,r0,r1
	mls r0,r2,r1,r0
	sub r0,r1,r0
mod_asm_neg5_exit:
	mov pc,lr
*/

/*
	ldr r1,=#MODULUS;
	cmp r0,	#0
	blt mod_asm_neg5
	//out=a-a/MODULUS
	sdiv r2,r0,r1
	mls r0,r2,r1,r0
	mov pc,lr
mod_asm_neg5:
	//num32=umod_asm(-num16);
	//num32=7681-num32;
	rsb r0,r0,#0
	sdiv r2,r0,r1
	mls r0,r2,r1,r0
	sub r0,r1,r0
	mov pc,lr
*/


	/****************UMOD_ASM_SIMD***********************\
	* Input: A
	* MODULUS: D
	* Output: mod(R0_low_word) + (mod(R0_high_word)<<16))
	* Overwrites r1-r5
	****************************************************/
	//ldr 	D,	=#UMOD_CONSTANT;
	#define UMOD_ASM_SIMD(A,B,C,D,E,F)\
		uxth 	B,	A;\
		umull	C, E, D, B;\
		lsrs	C, E, #11;\
		lsls	E, C, #4;\
		subs	E, E, C;\
		add		E, C, E, lsl #9;\
		subs	F, B, E;\
		uxth 	B,	A, ror	#16;\
		umull	C, E, D, B;\
		lsrs	C, E, #11;\
		lsls	E, C, #4;\
		subs	E, E, C;\
		add		E, C, E, lsl #9;\
		subs	C, B, E;\
		add 	A,	F,	C, lsl #16;	//Accumulate low and high results

		.global umod_asm_simd
		.extern umod_asm_simd
		.type umod_asm_simd, %function
	umod_asm_simd:
		stmdb	sp!, {r4, r5, lr}

		 //r0=input
		 //r3=magic number (UMOD_CONSTANT)
		 //r5=result of low part
		 //r0=output

		 ldr 	r3,	=#UMOD_CONSTANT

		 //Low part
		 uxth 	r1,	r0
		 umull	r2, r4, r3, r1 //r2,r4 = magic(r3)*num_low(r1) (result stored in r2 is never used)
		 lsrs	r2, r4, #11
		 lsls	r4, r2, #4
		 subs	r4, r4, r2
		 add	r4, r2, r4, lsl #9
		 subs	r5, r1, r4 //Store low result in r5

		 //High part
		 uxth 	r1,	r0, ror	#16
		 umull	r2, r4, r3, r1 //r2,r4 = magic(r3)*num_high(r1) (result stored in r2 is never used)
		 lsrs	r2, r4, #11
		 lsls	r4, r2, #4
		 subs	r4, r4, r2
		 add	r4, r2, r4, lsl #9
		 subs	r3, r1, r4 //Store high result in r3

		 add 	r0,	r5,	r3, lsl #16	//Accumulate low and high results

		 ldmia.w	sp!, {r4, r5, pc}



		/*
			.global bitreverse_asm
			.extern bitreverse_asm
			.type bitreverse_asm, %function

		bitreverse_asm:
			//void coefficient_add(uint32_t * a[M/2], uint32_t * b[M/2], uint32_t * c[M/2]);
			stmdb	sp!, {r4, lr}

			//r0 = *a
			//r1 = i
			//r2=swp_index
			//r3 a[2*i],a[2*i+1]
			//r4=a[2*swp_index],a[2*swp_index+1]

			mov r1,#1
			//**************Start of rearrange_loop****************
			//for(i=0; i<128; i++)
		bitreverse_loop:

			rbit r2,r1//r2=swp_index = bit1*64 + bit2*32 + bit3*16 + bit4*8 + bit5*4 + bit6*2 + bit7;
			lsr r2,#25

			//if(swp_index>i)
			cmp r2,r1
			ble rearrange_inner_skip

			lsr


			//u1 = a[2*i];
			//u2 = a[2*i+1];
			ldr r3,[r0,r1,lsl #2] //r3 = [a[2*i]], a[2*i+1]]

			//a[2*i] = a[2*swp_index];
			//a[2*i+1] = a[2*swp_index+1];
			ldr r4,[r0,r2,lsl #2] //r4 = [a[2*swp_index]], a[2*swp_index+1]]
			str r4,[r0,r1,lsl #2] //[a[2*i]], a[2*i+1]] = [a[2*swp_index]], a[2*swp_index+1]]
			str r3,[r0,r2,lsl #2] //[a[2*swp_index]], a[2*swp_index+1]] = [u1,u2]

		rearrange_inner_skip:
			add r1,#1// j(r7)+=2
			cmp r1,#128
			blt bitreverse_loop
			//**************End of rearrange_loop*****************

			ldmia.w	sp!, {r4, pc}
		*/

#ifdef PERFORM_MODULO_TESTS
	xputs("umod_asm:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		num16=(rand());
		if (umod_asm(num16)!=num16%7681)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("umod_div_asm:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		//num16=(rand());
		num16=i;
		if (umod_div_asm(num16)!=num16%7681)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");


	xputs("smod_asm:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		srand(i);
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;

		if (rnd1<rnd2)
			num16=rnd1-rnd2;
		else
			num16=(rnd2-rnd1);

		num32=smod_asm(num16);
		//num32=num16%7681;
		if (num16<0)
			num32=num32+7681;
		num16=mod(num16);
		if (num32!=num16)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("mod_asm2:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		srand(i);
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;

		if (rnd1<rnd2)
			num16=rnd1-rnd2;
		else
			num16=(rnd2-rnd1);

		num32=mod_asm2(num16);
		num16=mod(num16);
		if (num32!=num16)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("mod_asm3:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		srand(i);
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;

		if (rnd1<rnd2)
			num16=rnd1-rnd2;
		else
			num16=(rnd2-rnd1);


		num32=mod_asm3(num16);
		num16=mod(num16);
		if (num32!=num16)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("mod_asm4:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		srand(i);
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;

		if (rnd1<rnd2)
			num16=rnd1-rnd2;
		else
			num16=(rnd2-rnd1);

		num32=mod_asm4(num16);
		num16=mod(num16);
		if (num32!=num16)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("mod_asm5:");
	fail=0;
	for (i=-50000; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		srand(i);
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;


		num16=rnd1-rnd2;
		//num16=i;

		num32=mod_asm5(num16);
		num16=mod(num16);
		if (num32!=num16)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("mod_asm:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		srand(i);
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;

		if (rnd1<rnd2)
			num16=(rnd1-rnd2);
		else
			num16=(rnd2-rnd1);

		rnd = mod_asm(num16);
		rnd2 = mod(num16);

		if (rnd!=rnd2)
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");

	xputs("mod_asm_simd:");
	fail=0;
	for (i=0; ((i<SMALL_TEST_LOOPS) && (fail==0)); i++)
	{
		rnd1=rand()&0xffff;
		rnd2=rand()&0xffff;

		num1=umod_asm_simd(rnd1+(rnd2<<16));

		if ((mod(rnd1) != (num1&0xffff)) || (mod(rnd2) != ((num1>>16)&0xffff)))
		{
			fail=1;
			break;
		}
	}
	if (fail==1)
		xprintf("FAIL i=%x\n",i);
	else
		xprintf("OK\n");
#endif
