li $t1, 0
li $t2, 0
bgtz $t1, end1
addi $t1, $t1, 1
bgtz $t1, end2
end1: addu $t5, $t1, $t2 
end2: addu $t6, $t1, $t2
j jtarget
li $t4, 100
jtarget: li $t3, 42