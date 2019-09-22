	lw 1 0 one #add testing
	lw 2 0 two
	add 3 1 2
	add 4 3 1
	add 4 3 2
	add 4 3 3
	lw 7 0 four
	lw 3 0 five
	add 3 1 2
	nand 5 7 3
	nand 6 7 3
	nand 7 7 3
	lw 3 0 four
	lw 4 0 four
	add 3 3 5
	beq 3 4 fin
	beq 3 4 fin
	beq 3 4 fin
	add 3 3 7
	lw 1 3 one
	lw 1 3 two
	lw 1 3 three
	add 3 3 7
	sw 3 3 one
	sw 3 3 two
	sw 3 3 three
fin	halt
one .fill 1
two .fill 2
three .fill 3
four .fill 4
five .fill 5
six .fill 6
seven .fill 7
eight .fill 8