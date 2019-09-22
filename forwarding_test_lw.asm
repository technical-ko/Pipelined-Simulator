	lw 1 0 one #lw testing, tests both forwarding and NOOP injections
	lw 2 0 two 
	add 3 1 2 #tests MEMWB -> IDEX, WBEND -> IDEX: reg 3 should hold 3
	lw 1 0 three
	lw 2 0 four
	nand 3 1 2 #tests MEMWB -> IDEX, WBEND -> IDEX: reg 3 should hold -1
	lw 1 0 five
	lw 2 0 five
	beq 1 2 branch #tests MEMWB -> IDEX, WBEND -> IDEX: branch should occur
	halt	
branch lw 1 0 four
	lw 3 1 two #load value from datamem[*two + 4]. Tests MEMWB -> IDEX: reg 3 should hold 6
	lw 4 1 three #load value from datamem[*three + 4]. Tests WBEND -> IDEX: reg 4 should hold 7 
	lw 1 0 one #reg1 should hold 1
	sw 1 1 one # Tests MEMWB -> IDEX: memory address at label 'two' should hold 1
	sw 1 1 two #WBEND -> IDEX: memory address at label 'three' should now hold 1
	halt # instructions fetched should be 18, instructions retired should be 16
one .fill 1
two .fill 2
three .fill 3
four .fill 4
five .fill 5
six .fill 6
seven .fill 7
eight .fill 8