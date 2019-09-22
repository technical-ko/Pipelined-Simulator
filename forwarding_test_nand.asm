	lw 1 0 three #nand testing
	lw 2 0 four 
	lw 3 0 eight
	lw 5 0 eight
	lw 6 0 eight
	lw 7 0 eight
	nand 4 1 2
	add 3 3 4
	add 3 3 4
	add 3 3 4
	add 3 0 4
	add 4 0 0
	nand 4 1 2
	nand 5 3 4
	nand 6 3 4
	nand 7 3 4
	add 4 0 0
	nand 4 1 2
	beq 0 4 fin
	beq 0 4 fin
	beq 0 4 fin
	add 4 0 0
	add 5 0 0
	nand 4 1 2
	lw 5 4 six
	lw 6 4 seven
	lw 7 4 eight
	add 4 0 0
	nand 4 1 2
	sw 4 4 eight
	sw 4 4 seven
	sw 4 4 six
fin    halt
one .fill 1
two .fill 2
three .fill 3
four .fill 4
five .fill 5
six .fill 6
seven .fill 7
eight .fill 8