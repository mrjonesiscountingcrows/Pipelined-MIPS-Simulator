li $t1, 0    #t1 gets 0
li $t2, 0   #t2 gets 0
bgtz $t1, end1  #Data Hazard but no branch
addi $t1, $t1, 1  #t1 gets 1
bgtz $t1, end2      #Data Hazard and branch to end2
end1: addu $t5, $t1, $t2 
end2: addu $t6, $t1, $t2   #t6 gets 1
j jtarget                   #j to jtarget
li $t4, 100
jtarget: li $t3, 42         #t3 gets 2a hex which is 42 dec