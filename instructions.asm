start:
addi $t0, $0, 20		#Result is 20 in $t0
add $t1, $t0, $0		#Result is 20 in $t1
sub $t2, $t1, $0		#Result is 20 in $t2
slt $t3, $0, $t1		#Result is 1 in $t3
bne $t2,$t3, L1			#Result is branch to L1

L1:
addi $t3, $0, 20		#Result is 20 in $t3

beq $t2, $t3, L2		#Result is branch to L2

L2:
bgtz $t2, L3			#Result is Branch to L3

L3:
addi $t4, $0,1			#Result is 1 in $t4
ori $t4, 15			#Result is 15 in $t4

lui $t5, 1			#Result is 10000 in $t5

lui $t1, 0x1001			#Result in $t1 is 10010000
ori $t1, $t1, 0x0000		#Result in $t1 is 10010000
addi $t2, $0, 10		#Result in $t2 is 10
sw $t2, 0($t1)			#Store 10 in 10010000 memory
lw $t3, 0($t1)			#Retrieve 10 from 10010000 memory

addi $t0, $0, -12		#Result in $t0 is -12
addu $t5, $0, $t0		#Result in $t5 is -12 with negative flag

subu $t6, $t3,$t5		#Result is 22

sltu $t0, $t5, $t6		#Result is 0 in $t0

slti $t0, $t6, 35		#Result is 0 in $t0

j start				#Goes to start
