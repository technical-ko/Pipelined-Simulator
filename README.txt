Authors:
Keith O'Neal and Danny Mahota

This is a pipelined UST-3400 behavioral simulator that takes in UST-3400 machine code and prints out the state of the machine 
after each clock cycle as output.  

To run, run the Makefile and then pass the simulator your machine code using this commandline in linux:

./simulator -i your_machine_code.asm


simulator.c
^holds the simulator c code.


Makefile
^a makefile for simulator.c


overview.pdf
^an overview document that describes how the simulator works and our process in building it.


Annotated versions of each of the test files can be found in the annotated_tests folder. These files contain a copy of the assembly code
and include extensive comments that describe what each line is testing, as well as a pass/fail recording of our test results.
(NOTE: you may run into problems if you try to run these files through your assembler due to how long the comment fields are. 
They are intended only to assist in understanding what is going on in each test, not to be used for testing. For testing, use
the corresponding files in the test_cases folder.)

Within test_cases:

branch_test.asm
^tests how instructions fetched, total branches executed, and total branch mispredictions are affected by a
branch, as well as forwarding MEMWB -> IDEX, WBEND -> IDEX for lw -> beq.

forwarding_test_add.asm
^Tests forwarding for EXMEM -> IDEX, MEMWB -> IDEX, WBEND -> IDEX, for each of the following cases:
 add -> add
 add -> nand
 add -> beq
 add -> lw
 add -> sw


forwarding_test_lw.asm
^Tests NOOP injections and forwarding for MEMWB -> IDEX, WBEND -> IDEX, for each of the following cases:
 lw -> add
 lw -> nand
 lw -> beq
 lw -> lw
 lw -> sw


forwarding_test_nand.asm
^Tests forwarding for EXMEM -> IDEX, MEMWB -> IDEX, WBEND -> IDEX, for each of the following cases:
 nand -> add
 nand -> nand
 nand -> beq
 nand -> lw
 nand -> sw


forwarding_test_sw.asm
^Tests forwarding for WBEND -> IDEX, for each of the following cases:
 sw -> lw
