addi $t0, $0, 10
addi $t1, $0, 20
addi $t2, $0, 10
addi $t3, $0, 15
addi $t4, $0, 25
bne $t0, $t1, L1
add $t2, $t2,$t1
addi $t7, $0, 50
addi $t6, $0, 100
L1:
sub $t6,$t1,$t0
