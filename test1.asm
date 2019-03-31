li $t1, 69     #t1 gets 45 hex which is 69 dec
li $t2, 351     #t2 gets 15f which is 351 dec
slt $t3, $t2, $t1   #t3 is 0 after hazards
slt $t4, $t1, $t2   #t4 is 1 after hazards
sltu $t5, $t2, $t1  #t5 is 0
sltu $t6, $t1, $t2  #t6 is 1
slti $t7, $t1, 42   #t7 is 0
slti $s1, $t1, 100  #s1 is 1
