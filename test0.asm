
	addi  $t1,$zero,10				#t1 gets 10
	#pseudocode code : need lui and ori
	la    $t0, 268435456       # load address 0x10000000 #t0 gets 0x10000000
	sw    $t1, 0($t0)	#save word #Word Saved
	lw    $t2, 0($t0)	#load word #t2 gets 10
	addi  $t5, $t5, 8	#t5 gets 8
	addi  $t6, $t6, 6	#t6 gets 6
	slt   $t7, $t5, $t6	#t7 gets 0
	bne   $t5, $t6, L2	#Branch to L2
   L1:  addiu $t4, $t4, 10 	#t4 gets 10 after jump 
        addiu $t4, $t4, 5567  #t4 becomes 15c9
        j     exit			#Program ends
   L2:  add   $t8, $t8, $t6  #t8 gets 6
   	j     L1	
     	sub   $t3, $t3, $t6
   exit:
   
