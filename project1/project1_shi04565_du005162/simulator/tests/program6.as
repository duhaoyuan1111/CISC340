error 	lw    	1     	0     	one   	# load reg1 with 1 (symbolic address)
      	lw    	2     	0     	three 	# load reg2 with 3 (symbolic address)
      	nand  	3     	1     	2     	# nand reg1 and reg2, save answer(-2) in reg3
      	sw    	3     	5     	14     	# store reg3 in reg5's contents(address) plus 14
      	lw    	6     	0     	nine  	# load reg6 with 9 (symbolic address)
      	jalr  	4     	6             	# reg4 saves PC+1=6, jump to reg6's address which is 9
      	noop
      	noop
      	noop
      	beq   	1     	2     	error 	# if reg1 and reg2 are equal, go back to beginning (NO HALT ERROR)
one   	.fill 	1
three 	.fill 	3
nine  	.fill 	9
        .fill   0
stAdd 	.fill 	error					# will contain the address of error(0)