import sys

from subprocess import call
call(["make", "disassemble"])


starts="RLWE_enc_asm","RLWE_dec_asm","coefficient_add_asm","coefficient_mul_asm","coefficient_sub_asm","inv_ntt_asm","fwd_ntt_asm","fwd_ntt_parallel_asm","rearrange_asm","a_gen_asm","r2_gen_asm","encode_message_asm","r1_gen_asm","key_gen_asm","knuth_yao_asm","coefficient_mul_add_asm"
ends="message_gen_asm","RLWE_enc_asm","coefficient_mul_asm","coefficient_sub_asm","inv_ntt_asm","fwd_ntt_asm","fwd_ntt_parallel_asm","rearrange_asm","a_gen_asm","r2_gen_asm","encode_message_asm","r1_gen_asm","key_gen_asm","RLWE_dec_asm","umod_asm","coefficient_sub_asm"

def check(identifier):
	datafile = file('main.elf.dump')
	for line in datafile:
		  if identifier in line:
				return line.split(" ",1)[0]

for i in range(0, len(starts)):
	pos1 = check("<"+starts[i]+">:");
	pos2 = check("<"+ends[i]+">:");

#	print pos1
#	print pos2
	print starts[i] + ": " + str(int(pos2,16)-int(pos1,16))
