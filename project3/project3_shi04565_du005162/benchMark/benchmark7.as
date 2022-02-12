 lw 1 0 a
 lw 5 0 a
 beq 5 1 here
 add 1 0 5
 add 1 1 5
here sw 5 0 save
 halt
a .fill 12345
save .fill 0 # mem[8] = 12345
