#include "global.h"

#ifdef USE_TRNG
	#define GET_RAND_BASIC\
		ldr	r0, =#RNG_ADDR;\
	 	ldr	r0, [r0, #8];
#else
	#define GET_RAND_BASIC\
		push {r1,r2,r3,lr};\
		bl	rand;\
		pop {r1,r2,r3,lr};
#endif

/* Computes mod(A) by checking if A is greater than modulus, and subtracting the modulus if necessary
 * A = input/output
 * B = MODULULS
 */
#define UMOD_ADD_BASIC(A,B)\
	cmp A,B					/*check if A is greater than modulus*/;\
	it pl;\
	subpl A,A,B;			/*subtract modulus from A*/

/* Computes mod(A-B) by checking if A-B negative, and then adding the modulus if necessary
 * A,B = inputs
 * C = output
 * D = MODULULS
 */
#define SMOD_SUB_BASIC(A,B,C,D)\
	subs C,A,B				/*C = A-B*/;\
	it mi					/*SMOD*/;\
	addmi C,C,D;

  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.section .text

  .balign  2
  .thumb
  .thumb_func
  .code 16

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

#define UMOD_DIV_ASM(IN,MODREG,TMP)\
	udiv TMP,IN,MODREG;\
	mls IN,MODREG,TMP,IN

	.global umod_div_asm
	.extern umod_div_asm
	.type umod_div_asm, %function
umod_div_asm:
	ldr r1,=#MODULUS
	//udiv r2,r0,r1
	//mls r0,r1,r2,r0
	UMOD_DIV_ASM(r0,r1,r2)
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

/*********MOD_DIV_ASM*********
* Input: r0
* MODREG: Register containing #MODULUS
* Output: r2
*/
#define SMOD_DIV_ASM(IN,MODREG,OUT)\
	cmp IN,	#0;\
	it lt;\
	rsblt IN,IN,#0;\
	udiv OUT,IN,MODREG;\
	mls OUT,OUT,MODREG,IN;\
	it lt;\
	sublt OUT,MODREG,OUT

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

	.global coefficient_add_asm
	.extern coefficient_add_asm
	.type coefficient_add_asm, %function

coefficient_add_asm:
	//void coefficient_add(uint32_t * a[M/2], uint32_t * b[M/2], uint32_t * c[M/2]);
	//r0=a
	//r1=b
	//r2=c
	//r3=index
	//r4,r5,r6=tmp
	//r7=MODULUS
	stmdb	sp!, {r4, r5, r6, r7, lr}

	ldr 	r7,	=#MODULUS; //MODULUS_CONSTANT

/************Loop Start************/
	//for(j=0; j<M/2; j++)
	mov r3,#0 //index=r3

coefficient_add_loop:
	ldr r4,[r1,r3,lsl #2];
	ldr r5,[r2,r3,lsl #2];
	uadd16 r4,r4,r5

	uxth r6,r4
	UMOD_ADD_BASIC(r6,r7)

	lsr r4,r4,#16
	UMOD_ADD_BASIC(r4,r7)

	add r4,r6,r4,lsl #16 //Combine the results
	str r4, [r0,r3,lsl #2]

	add r3,#1
	cmp r3,#(M/2)
	bne coefficient_add_loop
/************Loop End************/

	ldmia.w	sp!, {r4, r5, r6, r7, pc}

	.global coefficient_mul_asm
	.extern coefficient_mul_asm
	.type coefficient_mul_asm, %function

coefficient_mul_asm:
	//void coefficient_add(uint32_t * a[M/2], uint32_t * b[M/2], uint32_t * c[M/2]);
	//r0=a
	//r1=b
	//r2=c
	//r3=index
	//r4=a[j]
	//r5=b[j]
	//r6,r7,r8,r9=tmp
	//r10=MODULUS_CONSTANT
	stmdb	sp!, {r4, r5, r6, r7, r8, lr}

	ldr 	r8,=#MODULUS; //MODULUS_CONSTANT

/************Loop Start************/
	//for(j=0; j<M/2; j++)
	mov r3,#0 //index=r3

coefficient_mul_loop:
	ldr r4,[r1,r3,lsl #2];
	ldr r5,[r2,r3,lsl #2];

	//Need to perform two multiplications as there is no instruction to perform a dual 16-bit multiply (without accumulation)
	smultt r6, r4, r5
	UMOD_DIV_ASM(r6,r8,r7)

	smulbb r4, r4, r5
	UMOD_DIV_ASM(r4,r8,r7)

	add r4,r4,r6,lsl #16
	str r4, [r0,r3,lsl #2]

	add r3,#1
	cmp r3,#(M/2)//cmp r3,#128
	bne coefficient_mul_loop
/************Loop End************/

	ldmia.w	sp!, {r4, r5, r6, r7, r8, pc}

	.global coefficient_mul_add_asm
	.extern coefficient_mul_add_asm
	.type coefficient_mul_add_asm, %function

coefficient_mul_add_asm:
	//void coefficient_add(uint32_t * a[M/2], uint32_t * b[M/2], uint32_t * c[M/2], uint32_t * d[M/2]);
	//r0=a
	//r1=b
	//r2=c
	//r3=d
	//r4=a[j]
	//r5=b[j]/c[j]
	//r6=index
	//r6,r7,r8,r9=tmp
	//r10=MODULUS_CONSTANT
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, r10, lr}

	ldr 	r8,=#MODULUS; //MODULUS_CONSTANT

/************Loop Start************/
	//for(j=0; j<M/2; j++)
	mov r10,#0 //index=r3

coefficient_mul_add_loop:
	ldr r4,[r1,r10,lsl #2];
	ldr r5,[r2,r10,lsl #2];
	ldr r9,[r3,r10,lsl #2];

	//Need to perform two multiplications as there is no instruction to perform a dual 16-bit multiply (without accumulation)
	smultt r6, r4, r5
	add r6,r6,r9,lsr #16
	UMOD_DIV_ASM(r6,r8,r7)

	smulbb r4, r4, r5
	uxth r9,r9
	add r4,r9
	UMOD_DIV_ASM(r4,r8,r7)

	add r4,r4,r6,lsl #16
	str r4, [r0,r10,lsl #2]

	add r10,r10,#1
	cmp r10,#(M/2) //cmp r10,#128
	bne coefficient_mul_add_loop
/************Loop End************/

	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, pc}

	.global coefficient_sub_asm
	.extern coefficient_sub_asm
	.type coefficient_sub_asm, %function

coefficient_sub_asm:
	//void coefficient_sub_asm(uint32_t * a[M/2], uint32_t * b[M/2], uint32_t * c[M/2]);
	//r0=a
	//r1=b
	//r2=c
	//r3=index
	//r4=a[j]
	//r5=b[j]
	//r6,r7 = temp
	//r8=MODULUS
	stmdb	sp!, {r4, r5, r6, r7, r8}

	ldr 	r8,=#MODULUS

/************Loop Start************/
	//for(j=0; j<M/2; j++)
	mov r3,#0 //index=r3

coefficient_sub_loop2:
	ldr r4,[r1,r3,lsl #2];
	ldr r5,[r2,r3,lsl #2];

	ssub16 r6,r4,r5

	sxth r4, r6
	SMOD_DIV_ASM(r4,r8,r7);//r7=a_low-b_low

	sxth r5, r6, ror #16
	SMOD_DIV_ASM(r5,r8,r4);//r4=a_high-b_high

	//Add the results together
	add r4, r7, r4, lsl #16
	str r4, [r0,r3,lsl #2]

	add r3,#1
	cmp r3,#(M/2)//cmp r3,#128
	bne coefficient_sub_loop2
/************Loop End************/

	ldmia.w	sp!, {r4, r5, r6, r7, r8}
	mov pc,lr

	#define INV_NTT_BASIC\
	ldr r3,[r0,r9,lsl #2] 			/*r3=a[2*(j+k)+1]],a[2*(j+k)]*/;\
	uxth r10,r3						/*u1(r10)=a[2*(j+k)]*/;\
	smultb r11,r3,r8 				/*t1=omega * a[2*(j+k)+1]*/;\
	/*r3=free, r9=j+k, r10=u1, r11=t1=omega * a[2*(j+k)+1], r12=free, r14=free*/;\
	UMOD_DIV_ASM(r11,r1,r12)		/*t1(r11)=mod(t1)*/;\
	add r12,r10,r11 				/*r12=u1+t1*/;\
	UMOD_ADD_BASIC(r12,r1) 			/*r12=mod(u1+t1)*/;\
	SMOD_SUB_BASIC(r10,r11,r14,r1)  /*r14=mod(u1-t1)*/;
	/*r3=free, r9=j+k, r10=free, r11=free, r12=mod(u1+t1)=a[2*(k+j)], r14=mod(u1-t1)=a[2*(k+j+m/2)];*/

	#define INV_NTT_SECOND_HALF\
	add r9,r9,r2,lsr #1				/*r9=j+k+m/2*/;\
	ldr r3,[r0,r9,lsl #2] 			/*r3=a[2*(j+k+m)+1]],a[2*(j+k+m)]*/;\
	smultb r11,r3,r8 				/*t2=omega * a[2*(j+k+m)+1]*/;\
	uxth r3,r3						/*r3=u2*/;\
	UMOD_DIV_ASM(r11,r1,r10);		/*t2(r11)=mod(t2)*/;\
	/*r3=u2, r9=j+k+m/2, r10=free, r11=t2, r12=mod(u1+t1)=a[2*(k+j)], r14=mod(u1-t1)=a[2*(k+j+m/2)]*/;\
	SMOD_SUB_BASIC(r3,r11,r10,r1);  /*r10=mod(u2-t2) -> a[2*(k+j+m/2)+1]*/;\
	add r10,r14,r10,lsl #16 		/*Combine*/;\
	/*r3=u2, r9=j+k+m/2, r10=a[2*(k+j+m/2)],a[2*(k+j+m/2)+1], r11=mod(t2), r12=mod(u1+t1), r14=free*/;\
	str r10,[r0,r9,lsl #2];\
	/*r3=u2, r9=free, r10=free, r11=t2, r12=mod(u1+t1), r14=free*/;\
	add r10,r3,r11 					/*r10=u2+t2*/;\
	UMOD_ADD_BASIC(r10,r1) 			/*r0=mod(u2-t2)-> a[2*(k+j)+1]*/;\
	add r12,r12,r10,lsl #16 		/*Combine a[2*(k+j)],a[2*(k+j)+1]*/;\
	/*r3=free, r9=free, r10=free, r11=free, r12=free, r14=a[2*(k+j)],a[2*(k+j)+1]*/;\
	add r9,r6,r7 					/*r9=j+k*/;\
	str r12,[r0,r9,lsl #2];\

	.global inv_ntt_asm
	.extern inv_ntt_asm
	.type inv_ntt_asm, %function
//void inv_ntt_asm(uint32_t a[]);
inv_ntt_asm:
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

	//r0: *a
	//r1: MODULUS
	//r2: m
	//r3: temp4
	//r4: primrt
	//r6: j
	//r7: k
	//r8: omega
	//r9: temp1
	//r10: u1/u2
	//r11: t1/t2
	//r12: temp2
	//r14 (lr): temp3

	ldr r1,=#MODULUS

	ldr r3,=primrt_inv_omega_table// r3=@primrt_inv_omega_table
	push {r3}

	/**************Start of m loop*****************/
	mov r2,#2
	//for(m=2; m<=128; m=2*m)
inv_m_loop:
	pop {r3}
	ldr r4,[r3]// primrt(r4)=primrt_omega_table[i],primrt_omega_table[i+1];
	add r3,#2 //Move pointer to primrt_omega_table one element onward
	push {r3}
	mov r8, #1//omega = 1;

/**************Start of j loop*****************/
	//for(j=0; j<m; j+=2)
	mov r6,#0// j(r6)=0
inv_j_loop:

	/**************Start of k loop*****************/
	//for(k=0; k<M; k=k+2*m)
	mov r7,#0
inv_k_loop:

	//Read {a[j+k+1], a[j+k]}
	add r9,r6,r7 			//r9=j(r6)+k(r7)

	INV_NTT_BASIC

	INV_NTT_SECOND_HALF

	add r7,r2 //k(r7) = k + m(r2)

	cmp r7,#(M/2)//cmp r7,#128
	blt inv_k_loop
	/**************End of k loop*****************/

	smulbb r8,r8,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r9)//omega = mod(omega);

	add r6,#1 //j(r6)+=1
	cmp r6,r2,lsr #1 //(j<m/2)
	bmi inv_j_loop
	/**************End of j loop*******************/

	lsl r2,#1 //m=2*m
	cmp r2,#(M/2)//cmp r2,#128
	//cmp r2,#4
	ble inv_m_loop
	/**************End of m loop*******************/

	//r9=j
#ifdef NTT512
	ldr r4, =#INVCONST1 //primrt = 2880;
#else
	mov r4, #INVCONST1 //primrt = 2880;
#endif
	mov r8, #1 //omega = 1;

	//for(j=0; j<M;)
	mov r9,#0
inv_ntt_j_loop2:
	INV_NTT_BASIC
	//r3=free, r9=index, r10=free, r11=free, r12=mod(u1+t1)=a[j], r14=mod(u1-t1)=a[j+1]

	add r12,r12,r14,lsl #16
	str r12,[r0,r9,lsl #2] 	//r10 = a[2*j],a[2*j+1]

	smulbb r8,r8,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r3)//omega = mod(omega);

	add r9,#1
	cmp r9,#(M/2)//cmp r9,#128
	blt inv_ntt_j_loop2
	/**************End of inv_ntt_j_loop2*******************/

	ldr r3, =#INVCONST2 	//r3=omega2 = 3383;
	ldr r4, =#INVCONST3 	//primrt = 2481;
	ldr r7, =#SCALING 	//7651;
	mov r8, #1 		//omega = 1;
	mov r9,#0 		//j=0
	//for(j=0; j<M;)
inv_ntt_j_loop3:
	ldr r10,[r0,r9,lsl #2] 	//r10 = a[j],a[j+1]

	smulbb r11,r10,r8 		//omega * a[j]
	UMOD_DIV_ASM(r11,r1,r12);//mod(omega * a[j])

	smulbb r11,r11,r7 		//omega * a[j] * 7651
	UMOD_DIV_ASM(r11,r1,r12);//mod(omega * a[j])

	smultb r14,r10,r3 		//omega2 * a[j+1]
	UMOD_DIV_ASM(r14,r1,r12);//mod(omega2 * a[j+1])

	smulbb r14,r14,r7 		//omega2 * a[j+1] * 7651
	UMOD_DIV_ASM(r14,r1,r12);//mod(omega * a[j])

	add r11,r11,r14, lsl #16 //r11 = a[j],a[j+1]
	str r11,[r0,r9,lsl #2]

	smulbb r8,r8,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r10)//omega = mod(omega);

	smulbb r3,r3,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r3,r1,r10)//omega = mod(omega);

	add r9,#1
	cmp r9,#(M/2)//cmp r9,#128
	blt inv_ntt_j_loop3
	/**************End of inv_ntt_j_loop2*******************/

early_end:
	pop {r3}
	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}


	.global fwd_ntt_asm
	.extern fwd_ntt_asm
	.type fwd_ntt_asm, %function
//void fwd_ntt_asm(uint32_t a[]);
fwd_ntt_asm:
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

	//r0: *a
	//r1: MODULUS
	//r2: m
	//r3: temp4
	//r4: primrt
	//r6: j
	//r7: k
	//r8: omega
	//r9: temp1
	//r10: u1/u2
	//r11: t1/t2
	//r12: temp2
	//r14 (lr): temp3

	ldr r1,=#MODULUS

	ldr r3,=primrt_omega_table// r3=@primrt_inv_omega_table
	push {r3} //Keep a pointer to primrt_omega_table. We will move the pointer later

	/**************Start of m loop*****************/
	mov r2,#2
	//for(m=2; m<=128; m=2*m)
fwd_m_loop:
	pop {r3}
	ldr r4,[r3]// primrt(r4)=primrt_omega_table[i],primrt_omega_table[i+1];
	uxth r8,r4,ror #16 //TODO: Rather than splitting these two, just keep them in one variable
					   //and use mulbt/mulbb etc to multiply
	uxth r4,r4
	add r3,#2 //Move pointer to primrt_omega_table one element onward
	push {r3}
	//mov r8, #1//omega = 1;

/**************Start of j loop*****************/
	//for(j=0; j<m; j+=2)
	mov r6,#0// j(r7)=0
fwd_j_loop:

	/**************Start of k loop*****************/
	//for(k=0; k<M; k=k+2*m)
	mov r7,#0
fwd_k_loop:

	//Read {a[j+k+1], a[j+k]}
	add r9,r6,r7 			//r9=j(r6)+k(r7)
	INV_NTT_BASIC
	//r3=free, r9=free, r10=free, r11=free, r12=mod(u1+t1)=a[2*(k+j)], r14=mod(u1-t1)=a[2*(k+j+m/2)]

	INV_NTT_SECOND_HALF

	add r7,r2 				//k(r7) = k + m(r2)

	//b early_end
	cmp r7,#(M/2)//cmp r7,#128
	blt fwd_k_loop
	/**************End of k loop*****************/

	smulbb r8,r8,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r9)//omega = mod(omega);

	add r6,#1// j(r6)+=1
	cmp r6,r2,lsr #1//(j<m/2)
	bmi fwd_j_loop
	/**************End of j loop*******************/

	lsl r2,#1 //m=2*m
	cmp r2,#(M/2)//cmp r2,#128
	//cmp r2,#4
	ble fwd_m_loop
	/**************End of m loop*******************/
	//b fwd_early_end

	ldr r4, =#FWD_CONST1		//primrt = 2880;
	ldr r8, =#FWD_CONST2		//omega = 1065;
	//for(j=0; j<M;)
	mov r9,#0 			//r9=j
fwd_ntt_j_loop2:
	INV_NTT_BASIC
	//r3=free, r9=index, r10=free, r11=free, r12=mod(u1+t1)=a[j], r14=mod(u1-t1)=a[j+1]

	add r12,r12,r14,lsl #16
	str r12,[r0,r9,lsl #2] 	//r10 = a[2*j],a[2*j+1]

	smulbb r8,r8,r4 		//omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r3)	//omega = mod(omega);

	add r9,#1
	cmp r9,#(M/2)//cmp r9,#128
	blt fwd_ntt_j_loop2
	/**************End of fwd_ntt_j_loop2*******************/

	pop {r3}
	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}

	.global fwd_ntt_parallel_asm
	.extern fwd_ntt_parallel_asm
	.type fwd_ntt_parallel_asm, %function
//void fwd_ntt_asm(uint32_t a[]);
fwd_ntt_parallel_asm:
	stmdb	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

	//r0: *a
	//r1: MODULUS
	//r2: m
	//r3: temp4
	//r4: primrt
	//r6: j
	//r7: k
	//r8: omega
	//r9: temp1
	//r10: u1/u2
	//r11: t1/t2
	//r12: temp2
	//r14 (lr): temp3

	ldr r1,=#MODULUS

	ldr r3,=primrt_omega_table// r3=@primrt_inv_omega_table
	push {r3} 			//Keep a pointer to primrt_omega_table. We will move the pointer later

	/**************Start of m loop*****************/
	mov r2,#2
	//for(m=2; m<=128; m=2*m)
fwd_parallel_m_loop:
	pop {r3}
	ldr r4,[r3]			// primrt(r4)=primrt_omega_table[i],primrt_omega_table[i+1];
	uxth r8,r4,ror #16 	//TODO: Rather than splitting these two, just keep them in one variable
					   	//and use mulbt/mulbb etc to multiply
	uxth r4,r4
	add r3,#2 			//Move pointer to primrt_omega_table to next element
	push {r3}

/**************Start of j loop*****************/
	//for(j=0; j<m; j+=2)
	mov r6,#0// j(r7)=0
fwd_parallel_j_loop:

	/**************Start of k loop*****************/
	//for(k=0; k<M; k=k+2*m)
	mov r7,#0
fwd_parallel_k_loop:

	//Read {a[j+k+1], a[j+k]}
	add r9,r6,r7 			//r9=j(r6)+k(r7)

	INV_NTT_BASIC
	//r3=free, r9=free, r10=free, r11=free, r12=mod(u1+t1)=a[2*(k+j)], r14=mod(u1-t1)=a[2*(k+j+m/2)]
	INV_NTT_SECOND_HALF

	//add r0,#0x200 			//Move pointer to fixed_data2
	add r0,#(M*2) 			//Move pointer to fixed_data2
	add r9,r6,r7 			//r9=j(r6)+k(r7)
	INV_NTT_BASIC
	//r3=free, r9=free, r10=free, r11=free, r12=mod(u1+t1)=a[2*(k+j)], r14=mod(u1-t1)=a[2*(k+j+m/2)]
	INV_NTT_SECOND_HALF

	//add r0,#0x200			//Move pointer to fixed_data3
	add r0,#(M*2)			//Move pointer to fixed_data3
	add r9,r6,r7 			//r9=j(r6)+k(r7)
	INV_NTT_BASIC
	//r3=free, r9=free, r10=free, r11=free, r12=mod(u1+t1)=a[2*(k+j)], r14=mod(u1-t1)=a[2*(k+j+m/2)]
	INV_NTT_SECOND_HALF
	//sub r0,#0x400			//Move pointer back to fixed_data1
	sub r0,#(M*4)			//Move pointer back to fixed_data1

	add r7,r2 //k(r7) = k + m(r2)

	//b early_end
	cmp r7,#(M/2)//cmp r7,#128
	blt fwd_parallel_k_loop
	/**************End of k loop*****************/

	smulbb r8,r8,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r9)//omega = mod(omega);

	add r6,#1// j(r6)+=1
	cmp r6,r2,lsr #1//(j<m/2)
	bmi fwd_parallel_j_loop
	/**************End of j loop*******************/

	lsl r2,#1 //m=2*m
	cmp r2,#(M/2)//cmp r2,#128
	//cmp r2,#4
	ble fwd_parallel_m_loop
	/**************End of m loop*******************/
	//b fwd_early_end

	ldr r4, =#FWD_CONST1 	//primrt = 5118;
	ldr r8, =#FWD_CONST2	//omega = 1065;
	//for(j=0; j<M;)
	mov r9,#0		//r9=j
fwd_parallel_ntt_j_loop2:
	INV_NTT_BASIC
	//r3=free, r9=index, r10=free, r11=free, r12=mod(u1+t1)=a[j], r14=mod(u1-t1)=a[j+1]
	add r12,r12,r14,lsl #16
	str r12,[r0,r9,lsl #2] 	//r10 = a[2*j],a[2*j+1]

	//add r0,#0x200
	add r0,#(M*2)
	INV_NTT_BASIC
	//r3=free, r9=index, r10=free, r11=free, r12=mod(u1+t1)=a[j], r14=mod(u1-t1)=a[j+1]
	add r12,r12,r14,lsl #16
	str r12,[r0,r9,lsl #2] 	//r10 = b[2*j],b[2*j+1]

	//add r0,#0x200
	add r0,#(M*2)
	INV_NTT_BASIC
	//r3=free, r9=index, r10=free, r11=free, r12=mod(u1+t1)=a[j], r14=mod(u1-t1)=a[j+1]
	add r12,r12,r14,lsl #16
	str r12,[r0,r9,lsl #2] 	//r10 = c[2*j],c[2*j+1]
	//sub r0,#0x400
	sub r0,#(M*4)

	smulbb r8,r8,r4 //omega(r8) = omega(16:0) * primrt(16:0) (r4);
	UMOD_DIV_ASM(r8,r1,r3)//omega = mod(omega);

	add r9,#1
	cmp r9,#(M/2)//cmp r9,#128
	blt fwd_parallel_ntt_j_loop2
	/**************End of fwd_ntt_j_loop2*******************/

fwd_early_end:
	pop {r3}
	ldmia.w	sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}

	//void rearrange_asm(uint32_t a);
	.global rearrange_asm
	.extern rearrange_asm
	.type rearrange_asm, %function

rearrange_asm:
	//void coefficient_add(uint32_t * a[M/2], uint32_t * b[M/2], uint32_t * c[M/2]);
	stmdb	sp!, {r4, lr}

	//r0 = *a
	//r1 = i
	//r2=swp_index
	//r3 a[2*i],a[2*i+1]
	//r4=a[2*swp_index],a[2*swp_index+1]

	mov r1,#1
	/**************Start of rearrange_loop*****************/
	//for(i=0; i<128; i++)
rearrange_loop:

	rbit r2,r1
#ifdef NTT512
	lsr r2,#24//r2=swp_index = bit1*128 + bit2*64 + bit3*32 + bit4*16 + bit5*8 + bit6*4 + bit7*2 + bit8;
#else
	lsr r2,#25//r2=swp_index = bit1*64 + bit2*32 + bit3*16 + bit4*8 + bit5*4 + bit6*2 + bit7;
#endif

	//if(swp_index>i)
	cmp r2,r1
	ble rearrange_loop_end

	//u1 = a[2*i];
	//u2 = a[2*i+1];
	ldr r3,[r0,r1,lsl #2] //r3 = [a[2*i]], a[2*i+1]]

	//a[2*i] = a[2*swp_index];
	//a[2*i+1] = a[2*swp_index+1];
	ldr r4,[r0,r2,lsl #2] //r4 = [a[2*swp_index]], a[2*swp_index+1]]

	str r4,[r0,r1,lsl #2] //[a[2*i]], a[2*i+1]] = [a[2*swp_index]], a[2*swp_index+1]]

	str r3,[r0,r2,lsl #2] //[a[2*swp_index]], a[2*swp_index+1]] = [u1,u2]

rearrange_loop_end:
	add r1,#1// j(r7)+=2
	cmp r1,#(M/2)//cmp r1,#128
	blt rearrange_loop
	/**************End of rearrange_loop*****************/

	ldmia.w	sp!, {r4, pc}


	//void a_gen_asm(uint32_t * a);
	.global a_gen_asm
	.extern a_gen_asm
	.type a_gen_asm, %function
a_gen_asm:

	stmdb	sp!, {r4, r5, r6, r7, lr}

	//r0=rand
	//r0-r3=temp
	//r4=UMOD_CONSTANT
	//r5=index i
	//r6=temp
	//r7=*a

	//ldr r4,=#UMOD_CONSTANT;
	ldr r4,=#MODULUS;
	mov r7,r0

	/**********************Loop Start********************/
	//for(i=0;i<256;i++)
	mov r5,#0
a_gen_loop:
	GET_RAND_BASIC
	//bl rand //uses r0-r3
	uxth r6,r0
	UMOD_DIV_ASM(r6,r4,r1);

	uxth r0,r0, ror #16
	UMOD_DIV_ASM(r0,r4,r1);

	add r6,r6,r0,lsl #16
	str r6,[r7,r5,lsl #2]

	add r5,#1
	cmp r5,#(M/2)//cmp r5,#128
	bne a_gen_loop

	mov r0,r7
	bl fwd_ntt_asm

	/**********************Loop End**********************/
	ldmia.w	sp!, {r4, r5, r6, r7, pc}

//r2_no_sign1:;
#define PROCESS_FOUR_BITS(A,B)\
	and r6,r0,#1;\
	lsrs r0,#2;\
	bcc A;\
	mul r6,r6,r7;\
A:;\
	and r8,r0,#1;\
	lsrs r0,#2;\
	bcc B;\
	mul r8,r8,r7;\
B:;\
	add r6,r6,r8,lsl #16;\
	str r6,[r4,r9,lsl #2];\
	add r9,#1;

//r2_no_sign2A:;

	//void a_gen_asm(uint32_t * a);
	.global r2_gen_asm
	.extern r2_gen_asm
	.type r2_gen_asm, %function
r2_gen_asm:

	stmdb	sp!, {r4, r5, r6, r7, r8, lr}

	//r0-r3=tmp
	//r4=*a
	//r5=index i
	//r6=num1
	//r7=#7680
	//r8=num2
	//r9=index j

	mov r4,r0
	//mov r7,#7680
	mov r7,#(MODULUS-1)

	/**********************Outer Loop(r4) Start*********************/
	//for(i=0;i<M;i++)
	mov r9,#0

r2_gen_loop:
	//bl rand //uses r0-r3
	GET_RAND_BASIC

	PROCESS_FOUR_BITS(id1,id2);
	PROCESS_FOUR_BITS(id3,id4);
	PROCESS_FOUR_BITS(id5,id6);
	PROCESS_FOUR_BITS(id7,id8);
	PROCESS_FOUR_BITS(id9,id10);
	PROCESS_FOUR_BITS(id11,id12);
	PROCESS_FOUR_BITS(id13,id14);
	PROCESS_FOUR_BITS(id15,id16);

	cmp r9,#(M/2)//cmp r9,#128

	//cmp r9,#16
	beq r2_gen_finish

	//TODO: Fix this mess?
	cmp r9,#(M/2)//cmp r9,#128
	bne r2_gen_loop
	/**********************Outer Loop End**********************/

r2_gen_finish:
	mov r0,r4
	bl fwd_ntt_asm
	ldmia.w	sp!, {r4, r5, r6, r7, r8, pc}
	mov pc,lr


	.global encode_message_asm
	.extern encode_message_asm
	.type encode_message_asm, %function

encode_message_asm:
	//r0=a
	//r1=
	//r2=QBY2
	//r3=a[index]
	//r5=index

	push {r4,r5}

	ldr 	r2,=#QBY2; //MODULUS_CONSTANT

/************Loop Start************/
	//for(j=0; j<M/2; j++)
	mov r5,#0 //index=r3

encode_message_loop:
	ldr r3,[r1,r5,lsl #2];
	smulbb r4,r3,r2
	smultb r3,r3,r2
	add r4,r4,r3, lsl #16
	str r4,[r0,r5,lsl #2];

	add r5,#1
	cmp r5,#(M/2)//cmp r5,#128
	bne encode_message_loop
/************Loop End************/

	pop {r4,r5}
	mov pc,lr

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

