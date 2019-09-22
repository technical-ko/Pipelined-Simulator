#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDstruct
{
	int instr;
	int pcplus1;
}IFIDType;


typedef struct IDEXstruct
{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
}IDEXType;



typedef struct EXMEMstruct
{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
}EXMEMType;


typedef struct MEMWBstruct
{
	int instr;
	int writedata;
}MEMWBType;

typedef struct WBENDstruct
{
	int instr;
	int writedata;
}WBENDType;


typedef struct stateStruct {
    int pc;
	int instrmem[NUMMEMORY];
	int datamem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles; /*Number of cycles fetched so far*/
	int fetched; /*Total number of instructions fetched*/
	int retired; /*Total number of completed instructions*/
	int branches; /*Total number of branches executed*/
	int mispreds; /*Number of branch mispredictions*/
} stateType;

int field0(int instruction){
    return( (instruction>>19) & 0x7);
}

int field1(int instruction){
    return( (instruction>>16) & 0x7);
}

int field2(int instruction){
    return(instruction & 0xFFFF);
}

int opcode(int instruction){
    return(instruction>>22);
}


int signExtend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}


void printinstruction(int instr) {
 char opcodestring[10];
 if (opcode(instr) == ADD) {
	strcpy(opcodestring, "add");
 } else if (opcode(instr) == NAND) {
	strcpy(opcodestring, "nand");
 } else if (opcode(instr) == LW) {
	strcpy(opcodestring, "lw");
 } else if (opcode(instr) == SW) {
	strcpy(opcodestring, "sw");
 } else if (opcode(instr) == BEQ) {
	strcpy(opcodestring, "beq");
 } else if (opcode(instr) == JALR) {
	strcpy(opcodestring, "jalr");
 } else if (opcode(instr) == HALT) {
	strcpy(opcodestring, "halt");
 } else if (opcode(instr) == NOOP) {
	strcpy(opcodestring, "noop");
 } else {
	strcpy(opcodestring, "data");
 }
 if(opcode(instr) == ADD || opcode(instr) == NAND){
printf("%s %d %d %d\n", opcodestring, field2(instr), field0(instr), field1(instr));
 }else if(0 == strcmp(opcodestring, "data")){
printf("%s %d\n", opcodestring, signExtend(field2(instr)));
 }else{
printf("%s %d %d %d\n", opcodestring, field0(instr), field1(instr),
signExtend(field2(instr)));
 }
}



void printstate(stateType* stateptr){

 int i;
 printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
 printf("\tpc %d\n", stateptr->pc);
 printf("\tdata memory:\n");
for (i=0; i<stateptr->numMemory; i++) {
 printf("\t\tdatamem[ %d ] %d\n", i, stateptr->datamem[i]);
}
 printf("\tregisters:\n");
for (i=0; i<NUMREGS; i++) {
 printf("\t\treg[ %d ] %d\n", i, stateptr->reg[i]);
}
 printf("\tIFID:\n");
printf("\t\tinstruction ");
printinstruction(stateptr->IFID.instr);
printf("\t\tpcplus1 %d\n", stateptr->IFID.pcplus1);
 printf("\tIDEX:\n");
printf("\t\tinstruction ");
printinstruction(stateptr->IDEX.instr);
printf("\t\tpcplus1 %d\n", stateptr->IDEX.pcplus1);
printf("\t\treadregA %d\n", stateptr->IDEX.readregA);
printf("\t\treadregB %d\n", stateptr->IDEX.readregB);
printf("\t\toffset %d\n", stateptr->IDEX.offset);
 printf("\tEXMEM:\n");
printf("\t\tinstruction ");
printinstruction(stateptr->EXMEM.instr);
printf("\t\tbranchtarget %d\n", stateptr->EXMEM.branchtarget);
printf("\t\taluresult %d\n", stateptr->EXMEM.aluresult);
printf("\t\treadreg %d\n", stateptr->EXMEM.readreg);
 printf("\tMEMWB:\n");
printf("\t\tinstruction ");
printinstruction(stateptr->MEMWB.instr);
printf("\t\twritedata %d\n", stateptr->MEMWB.writedata);
 printf("\tWBEND:\n");
printf("\t\tinstruction ");
printinstruction(stateptr->WBEND.instr);
printf("\t\twritedata %d\n", stateptr->WBEND.writedata);
}


void print_stats(int n_instrs){
	printf("INSTRUCTIONS: %d\n", n_instrs);
}

void run(stateType* state){


	// Reused variables;
	int instr = 0;
    int regA = 0;
	int regB = 0;
	int offset = 0;
	int branchTarget = 0;
	int aluResult = 0;
	int total_instrs = 0;
	stateType* newstate = (stateType*)malloc(sizeof(stateType));
    
    int readReg = 0;
    int writeData = 0;

	state->IFID.instr = 0x1c00000;
	state->IDEX.instr = 0x1c00000;
	state->EXMEM.instr = 0x1c00000;
	state->MEMWB.instr = 0x1c00000;
	state->WBEND.instr = 0x1c00000;
	state->cycles = 0;

	// Primary loop
    while(1){

		printstate(state);

		if(HALT == opcode(state->MEMWB.instr)){
            
            state->fetched = state->fetched - 3;
            state->retired = state->fetched - (3 * state->mispreds);
            
			printf("machine halted \n");
			printf("total of %d cycles executed\n", state->cycles);
			printf("total of %d instructions fetched\n", state->fetched);
			printf("total of %d instructions retired\n", state->retired);
			printf("total of %d branches executed\n", state->branches);
			printf("total of %d branch mispredictions\n", state->mispreds);

		exit(0);
		}

		*newstate = *state;
        newstate->cycles++;
        newstate->fetched++;

/*------------------ IF stage ----------------- */
        
        instr = state->instrmem[state->pc];
        
        newstate->IFID.instr = state->instrmem[state->pc];
        newstate->pc = state->pc+1;
        newstate->IFID.pcplus1 = state->pc+1;


/*------------------ ID stage ----------------- */

        instr = state->IFID.instr;

        regA = 0;
        regB = 0;
        offset = 0;
        
        
		if(opcode(instr) == ADD || opcode(instr) == NAND)
        {
            regA = state->reg[field0(instr)];
            regB = state->reg[field1(instr)];
        }
        else if(opcode(instr) == LW || opcode(instr) == SW)
        {
            regB = state->reg[field1(instr)];
        }
        else if(opcode(instr) == BEQ)
        {
            regA = state->reg[field0(instr)];
            regB = state->reg[field1(instr)];
        }

		// Set sign extended offset
        if(opcode(instr) == LW || opcode(instr) == SW || opcode(instr) == BEQ){
            offset = signExtend(field2(instr));
        }

        
		//LOAD VALUES INTO BUFFER
	 	newstate->IDEX.instr = instr;
        newstate->IDEX.pcplus1 = state->IFID.pcplus1;
        newstate->IDEX.readregA = regA;
        newstate->IDEX.readregB = regB;
        newstate->IDEX.offset = offset;

/*------------------ EX stage ----------------- */

        
        instr = state->IDEX.instr;
        
        // *** NOOP INJECTION ***
        int injectNoOp = 0;
        
        if(opcode(state->EXMEM.instr) == LW)
        {
            // if next instruction is LW check for conflict
            if(opcode(instr) == ADD || opcode(instr) == NAND || opcode(instr) == BEQ)
            {
                if(field0(instr) == field0(state->EXMEM.instr)) injectNoOp = 1;
                if(field1(instr) == field0(state->EXMEM.instr)) injectNoOp = 1;

            }
            else if(opcode(instr) == SW || opcode(instr) == LW)
            {
                if(field1(instr) == field0(state->EXMEM.instr)) injectNoOp = 1;
            }
            
        }
        
        if(injectNoOp == 1) // is DataHazard
        {
            newstate->fetched = newstate->fetched - 1;
            
            *newstate = *state;
            newstate->cycles++;
            
            if(opcode(state->WBEND.instr) == LW)
            {
                int regWBEND = field0(state->WBEND.instr);
                int regNewValueWBEND = state->WBEND.writedata;
                
                if(opcode(instr) == ADD || opcode(instr) == NAND || opcode(instr) == BEQ)
                {
                    if(field0(instr) == regWBEND) newstate->IDEX.readregA = regNewValueWBEND;
                    if(field1(instr) == regWBEND) newstate->IDEX.readregB = regNewValueWBEND;
                }
                else if(opcode(instr) == SW || opcode(instr) == LW)
                {
                    if(field1(instr) == regWBEND) newstate->IDEX.readregB = regNewValueWBEND;
                }
            }
            
            //inject NOOP
            newstate->EXMEM.instr = NOOPINSTRUCTION;
            newstate->EXMEM.branchtarget = 0;
            newstate->EXMEM.aluresult = 0;
            newstate->EXMEM.readreg = 0;
        }
        else // not DataHazard
        {
        
            // *** FORWARDING ***
            
            //initialize the regA and regB local variables
            //NOTE: these values come from the pipeline
            regA = state->IDEX.readregA;
            regB = state->IDEX.readregB;
            
            //Initialize some other local variables...
            int regInUseA = -1;
            int regInUseB = -1;
            
            int regInProgressEXMEM = -1;
            int regInProgressMEMWB = -1;
            int regInProgressWBEND = -1;
            
            int regNewValueEXMEM = 0;
            int regNewValueMEMWB = 0;
            int regNewValueWBEND = 0;
            
            //Check what registers are needed in the EX stage (state->EXMEM.instr)
            if(opcode(instr) == ADD || opcode(instr) == NAND || opcode(instr) == BEQ || opcode(instr) == SW)
            {
                regInUseA = field0(instr);
                regInUseB = field1(instr);
            }
            else if(opcode(instr) == LW)
            {
                regInUseB = field1(instr);
            }
            
            //Check what registers are out of date
            //EXMEM Buffer
            if(opcode(state->EXMEM.instr) == ADD || opcode(state->EXMEM.instr) == NAND)
            {
                regInProgressEXMEM = field2(state->EXMEM.instr);
                regNewValueEXMEM = state->EXMEM.aluresult;
            }
            
            //MEMWB buffer
            if(opcode(state->MEMWB.instr) == ADD || opcode(state->MEMWB.instr) == NAND)
            {
                regInProgressMEMWB = field2(state->MEMWB.instr);
                regNewValueMEMWB = state->MEMWB.writedata;
            }
            else if(opcode(state->MEMWB.instr) == LW)
            {
                regInProgressMEMWB = field0(state->MEMWB.instr);
                regNewValueMEMWB = state->MEMWB.writedata;
            }
            
            //WBEND Buffer
            if(opcode(state->WBEND.instr) == ADD || opcode(state->WBEND.instr) == NAND)
            {
                regInProgressWBEND = field2(state->WBEND.instr);
                regNewValueWBEND = state->WBEND.writedata;
            }
            else if(opcode(state->WBEND.instr) == LW)
            {
                regInProgressWBEND = field0(state->WBEND.instr);
                regNewValueWBEND = state->WBEND.writedata;
            }
            
            
            
            //update values regA and regB if they are out of date
            if(regInUseA != -1)
            {
                if(regInProgressEXMEM == regInUseA) regA = regNewValueEXMEM;
                else if(regInProgressMEMWB == regInUseA) regA = regNewValueMEMWB;
                else if(regInProgressWBEND == regInUseA) regA = regNewValueWBEND;
                
            }
            
            if(regInUseB != -1)
            {
                if(regInProgressEXMEM == regInUseB) regB = regNewValueEXMEM;
                else if(regInProgressMEMWB == regInUseB) regB = regNewValueMEMWB;
                else if(regInProgressWBEND == regInUseB) regB = regNewValueWBEND;
            }
            
            
            // *** COMPUTE ALU and BRANCHING ***
            
            branchTarget = state->IDEX.pcplus1;
            aluResult = 0;
            readReg = 0;
            
            // ADD
            if(opcode(instr) == ADD){
                // Add
                aluResult = regA + regB;

            }
            // NAND
            else if(opcode(instr) == NAND){
                // NAND
                aluResult = ~(regA & regB);

            }
            // LW or SW
            else if(opcode(instr) == LW || opcode(instr) == SW){
                // Calculate memory address
                aluResult = regB + state->IDEX.offset;

                readReg = regA;
            }
            // BEQ
            else if(opcode(instr) == BEQ){
                
                branchTarget = state->IDEX.pcplus1 + state->IDEX.offset;
                aluResult = regA - regB;
            }
            
            //LOAD VALUES INTO BUFFER
            newstate->EXMEM.instr = instr;
            newstate->EXMEM.branchtarget = branchTarget;
            newstate->EXMEM.aluresult = aluResult;
            newstate->EXMEM.readreg = readReg;
        }


/*------------------ MEM stage ----------------- */

        writeData = 0;
        
        instr = state->EXMEM.instr;
       
        if(opcode(instr) == ADD || opcode(instr) == NAND){
            //Add or NAND
            writeData = state->EXMEM.aluresult;
        }
        else if(opcode(instr) == SW){
            // Store
            newstate->datamem[state->EXMEM.aluresult] = state->EXMEM.readreg;
        }else if(opcode(instr) == LW){
            // Load
            writeData = state->datamem[state->EXMEM.aluresult];
            
        }else if (opcode(instr) == BEQ){
            newstate->branches++;
            if(state->EXMEM.aluresult == 0){
                //set PC
                newstate->pc = state->EXMEM.branchtarget;
                
                //clear pipeline
                newstate->IFID.instr = NOOPINSTRUCTION;
                newstate->IFID.pcplus1 = 0;
                
                newstate->IDEX.instr = NOOPINSTRUCTION;
                newstate->IDEX.pcplus1 = 0;
                newstate->IDEX.readregA = 0;
                newstate->IDEX.readregB = 0;
                newstate->IDEX.offset = 0;
                
                newstate->EXMEM.instr = NOOPINSTRUCTION;
                newstate->EXMEM.branchtarget = 0;
                newstate->EXMEM.aluresult = 0;
                newstate->EXMEM.readreg = 0;
                newstate->mispreds++;
            }

        }
        
        newstate->MEMWB.instr = state->EXMEM.instr;
        newstate->MEMWB.writedata = writeData;

/*------------------ WB stage ----------------- */

        instr = state->MEMWB.instr;
        
        if(opcode(instr) == ADD || opcode(instr) == NAND){
            newstate->reg[field2(instr)] = state->MEMWB.writedata;
        }
        else if(opcode(instr) == LW){
            newstate->reg[field0(instr)] = state->MEMWB.writedata;
        }

		
		newstate->WBEND.instr = instr;
        newstate->WBEND.writedata = state->MEMWB.writedata;

		//DECODING
        *state = *newstate;
        
        if(state->cycles > 100)
        {
            printf("***** TEST CASE IS OVER 100 CYCLES *****");
            break;
        }
    
        
    } // While
    
	print_stats(total_instrs);
}

int main(int argc, char** argv){


	/** Get command line arguments **/
    char* fname;

	opterr = 0;

	int cin = 0;

	while((cin = getopt(argc, argv, "i:")) != -1){
		switch(cin)
		{
			case 'i':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';

				strncpy(fname, optarg, strlen(optarg)+1);
				printf("FILE: %s\n", fname);
				break;
			case '?':
				if(optopt == 'i'){
					printf("Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					printf("Unknown option `-%c'.\n", optopt);
				}
				else{
					printf("Unknown option character `\\x%x'.\n", optopt);
					return 1;
				}
				break;
			default:
				abort();
		}
	}

	/*
	if(argc == 1){
		fname = (char*)malloc(sizeof(char)*100);
		printf("Enter the name of the machine code file to simulate: ");
		fgets(fname, 100, stdin);
		fname[strlen(fname)-1] = '\0'; // gobble up the \n with a \0
	}
	else if (argc == 2){

	    int strsize = strlen(argv[1]);

		fname = (char*)malloc(strsize);
		fname[0] = '\0';

		strcat(fname, argv[1]);
	}else{
		printf("Please run this program correctly\n");
		exit(-1);
	}
	*/

	FILE *fp = fopen(fname, "r");

	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}

	/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
    while (EOF != (c=getc(fp))) {
        if ( c == '\n' ){
            line_count++;
		}
    }
	// reset fp to the beginning of the file
	rewind(fp);

	stateType* state = (stateType*)malloc(sizeof(stateType));

	state->pc = 0;
	memset(state->instrmem, 0, line_count);
	memset(state->datamem, 0, NUMMEMORY*sizeof(int));

	memset(state->reg, 0, NUMREGS*sizeof(int));

	state->numMemory = line_count;

	char line[256];

	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
        /* note that fgets doesn't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */

		state->instrmem[i] = atoi(line);
        state->datamem[i] = atoi(line);
		i++;
    }
    fclose(fp);

	/** Run the simulation **/
	run(state);

	free(state);
	free(fname);

}
