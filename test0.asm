
	addi  $t1,$zero,10
	#pseudocode code : need lui and ori
	la    $t0, 268435456       # load address 0x10000000
	sw    $t1, 0($t0)	#save word
	lw    $t2, 0($t0)	#load word
	addi  $t5, $t5, 8
	addi  $t6, $t6, 6
	slt   $t7, $t5, $t6
	bne   $t5, $t6, L2
   L1:  addiu $t4, $t4, 10
        addiu $t4, $t4, 5567
        j     exit
   L2:  add   $t8, $t8, $t6
   	j     L1
     	sub   $t3, $t3, $t6
   exit:
   
