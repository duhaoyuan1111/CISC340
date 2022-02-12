       lw    6      0      zero   	# load reg6 with 0 (counter)
       lw    1      0      machd 	# load reg1 with machd
       lw    2      0      mplier 	# load reg2 with mplier 	   
       lw    3      0      one 		# load reg3 with one
while  nand  4      0      2 		# nand mplier with reg0 
	   nand  5      3      2 		# nand mplier with reg3 
	   add   3      3      3 		# add reg3 with itself (nand op, 1,2,4,8 ...)
	   beq   4		5	   equal 	# compare nand values which stored in reg4 and reg5
	   add	 7		7	   1 		# store result into reg7
equal  add   1		1	   1 		# shift one bit left for machd
	   lw  	 4		0	   fif 		# load reg4 with fif(15)
	   lw 	 5		0	   one 		# load reg5 with one
	   add 	 6		6	   5 		# counter add 1
  	   beq 	 6		4	   done 	# compare reg4 and reg6, check if the reg6 is equal to 15(if the loop run 15 times)
	   beq	 0		0	   while 	# loop
done   halt 						# finish looping 
mplier .fill 11834
machd  .fill 29562
one	   .fill 1                		# will contain the address of start (2)
fif	   .fill 15
zero   .fill 0