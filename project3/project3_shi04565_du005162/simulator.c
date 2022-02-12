#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR – not implemented in this project */
#define HALT 6
#define NOOP 7
#define NOOPINSTRUCTION 0x1c00000
int signextend(int num){
// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}

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
		printf("%s %d\n", opcodestring, signextend(field2(instr)));
   }else{
		printf("%s %d %d %d\n", opcodestring, field0(instr), field1(instr),
		signextend(field2(instr)));
   }
}

typedef struct IFIDstruct{
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;

typedef struct MEMWBstruct{
	int instr;
	int writedata;
} MEMWBType;

typedef struct WBENDstruct{
	int instr;
	int writedata;
} WBENDType;


typedef struct state_struct {
	int pc;
	int instrmem[65536];
	int datamem[65536];
	int reg[8];
	int num_memory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles;       /* Number of cycles run so far */
	int fetched;     /* Total number of instructions fetched */
	int retired;      /* Total number of completed instructions */
	int branches;  /* Total number of branches executed */
	int mispreds;  /* Number of branch mispredictions*/
} statetype;

void printstate(statetype *stateptr){
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
    printf("\tpc %d\n", stateptr->pc);
    printf("\tdata memory:\n");
	for (i=0; i<stateptr->num_memory; i++) {
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

int power(int a){
	int result = 1;
	for(int i=0;i<a;i++){
		result = result*2;
	}
	return result;
}

int convert_num(int num){
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return(num);
}
	 
int main (int argc, char **argv){
	
FILE* mc;
char read[160];
char *ivalue = NULL;
char *ovalue = NULL;
int a;
while ((a = getopt (argc, argv, "i:")) != -1)
	switch (a)
		{
		case 'i': 
			ivalue=optarg;
			break;
		default:
			abort ();
		}
	int c, k;
	int j=0;
	char bina[65536][33];
	char deci[65536][12];
	for(int i=0;i<50;i++){
		memset(bina[i],'\0',sizeof(bina[i]));
		memset(deci[i],'\0',sizeof(deci[i]));
	}
	if(ivalue !=NULL){	
		mc=fopen(ivalue,"r");
		while(fgets(read,sizeof(read),mc)!= NULL){
			
			for(int i=0; i<12;i++){
				deci[j][i]=read[i];
			}
			int input=atoi(read);
			for (c = 31; c >= 0; c--){
				k = input >> c;
				if (k & 1)
					bina[j][31-c]='1';
				else
					bina[j][31-c]='0';
			}
			j=j+1;
		}
		fclose(mc);
		statetype state;//old state
		
		state.num_memory=j;
		memset(state.instrmem,0,sizeof(state.instrmem));
		memset(state.datamem,0,sizeof(state.datamem));
		memset(state.reg,0,sizeof(state.reg));
		
		state.cycles=0;       
		state.fetched=0;     
		state.retired=0;      
		state.branches=0;  
		state.mispreds=0;
		
		state.IFID.instr=29360128;
		state.IFID.pcplus1=0;
		
		state.IDEX.instr=29360128;
		state.IDEX.pcplus1=0;
		state.IDEX.readregA=0;
		state.IDEX.readregB=0;
		state.IDEX.offset=0;

		state.EXMEM.instr=29360128;
		state.EXMEM.branchtarget=0;
		state.EXMEM.aluresult=0;
		state.EXMEM.readreg=0;
	
		
		state.MEMWB.instr=29360128;
		state.MEMWB.writedata=0;
		
		state.WBEND.instr=29360128;
		state.WBEND.writedata=0;
		
		statetype newstate = state;//new state
		
		for(int i=0;i<j;i++){
			state.datamem[i]=atoi(deci[i]);
			state.instrmem[i]=atoi(deci[i]);
		}
		state.pc=0;
		
		int counter = 1;
		int check=0;
		int haltLine = 0;
		
		
		while(strcmp(deci[haltLine],"25165824\n")!=0){
			haltLine++;
			if(haltLine==j){
				printf("ERROR: <Halt instruction needed>\n");
				exit(-1);
			}
		}
		
		int lwaddLoadStall;
		
		int lwswLoadStall;
		int tempEXMEM;
		
		int tempWBEND;
		int tempinstrWBEND;
		
		int temp;
		int tempinstrMEMWB;
		while(1==1){
				
			printstate(&state);
			/* check for halt */
			if(HALT == opcode(state.MEMWB.instr)) {
				printf("machine halted\n");
				printf("total of %d cycles executed\n", state.cycles);
				printf("total of %d instructions fetched\n", state.fetched);
				printf("total of %d instructions retired\n", state.retired);
				printf("total of %d branches executed\n", state.branches);
				printf("total of %d branch mispredictions\n", state.mispreds);
				exit(0);
			}
			newstate = state;
			newstate.cycles++;
			/*------------------ IF stage ----------------- */
			newstate.fetched++;
			newstate.IFID.instr=state.datamem[state.pc];
			newstate.IFID.pcplus1=state.pc+1;
			
			if((opcode(state.IFID.instr)==6||(opcode(state.IDEX.instr)==6)||(opcode(state.EXMEM.instr)==6))){
				newstate.fetched--;
				newstate.retired--;
			}
			/*------------------ ID stage ----------------- */
			newstate.IDEX.instr =state.IFID.instr;
			newstate.IDEX.pcplus1=state.IFID.pcplus1;
			
			if((((newstate.IDEX.instr)>>22) == 0)||(((newstate.IDEX.instr)>>22) == 1)){//R type
				newstate.IDEX.readregA=state.reg[((state.IFID.instr)>>19)&0x7];
				newstate.IDEX.readregB=state.reg[((state.IFID.instr)>>16)&0x7];
				newstate.IDEX.offset =field2(newstate.IDEX.instr);
			}else if((((newstate.IDEX.instr)>>22) == 2)||(((newstate.IDEX.instr)>>22) == 3)|| (((newstate.IDEX.instr)>>22) == 4)){//i type
				newstate.IDEX.readregA=state.reg[((state.IFID.instr)>>19)&0x7];
				newstate.IDEX.readregB=state.reg[((state.IFID.instr)>>16)&0x7];
				newstate.IDEX.offset =signextend((state.IFID.instr)&0xFFFF);				
			}else{//O type
				newstate.IDEX.readregA=state.reg[((state.IFID.instr)>>19)&0x7];
				newstate.IDEX.readregB=state.reg[((state.IFID.instr)>>16)&0x7];
				newstate.IDEX.offset = 0;				
			}
			
			
			/*------------------ EX stage ----------------- */
			
			newstate.EXMEM.instr =state.IDEX.instr;
			
			int wbendOP=opcode(tempinstrWBEND);
			int wbendA = field0(tempinstrWBEND);
			int wbendB = field1(tempinstrWBEND);
			int wbendoff = field2(tempinstrWBEND);
			
			int memwbOP=opcode(tempinstrMEMWB);
			int memwbA = field0(tempinstrMEMWB);
			int memwbB = field1(tempinstrMEMWB);
			int memwboff = field2(tempinstrMEMWB);	
			
			
			if(((newstate.EXMEM.instr)>>22) == 0){//add
				
			
				
				if((((state.EXMEM.instr)>>22)==0) || (((state.EXMEM.instr)>>22)==1)){// add  1 2 3 
																					 // and  2 1 1
															 
					if(opcode(state.EXMEM.instr)==6){

						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;	
						newstate.EXMEM.aluresult=state.IDEX.readregA+state.IDEX.readregB;
						newstate.EXMEM.readreg=state.IDEX.readregA;							
						newstate.fetched--;
					}else if((((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7))&&(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7))){
						
						
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=tempEXMEM+tempEXMEM;
						newstate.EXMEM.readreg=tempEXMEM;	
						
					}else if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7)){
						
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>19)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=temp+tempEXMEM;
							newstate.EXMEM.readreg=temp;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>19)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempWBEND+tempEXMEM;
							newstate.EXMEM.readreg=tempWBEND;	

						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=state.IDEX.readregA+tempEXMEM;
							newstate.EXMEM.readreg=state.IDEX.readregA;	
						}
					}else if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>16)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=temp+tempEXMEM;
							newstate.EXMEM.readreg=tempEXMEM;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>16)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempWBEND+tempEXMEM;
							newstate.EXMEM.readreg=tempEXMEM;	

						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempEXMEM+state.IDEX.readregB;
							newstate.EXMEM.readreg=tempEXMEM;
						}							
					}else{
						newstate.EXMEM.aluresult=state.IDEX.readregA+state.IDEX.readregB;
						newstate.EXMEM.readreg=state.IDEX.readregA;
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;						
					}
				}else if(((state.EXMEM.instr)>>22)==2){
					
					if(field0(newstate.EXMEM.instr)== field0(state.EXMEM.instr) || field1(newstate.EXMEM.instr)== field0(state.EXMEM.instr)){
						newstate.IFID.instr=state.IFID.instr;
						newstate.IFID.pcplus1=state.IFID.pcplus1;	
						
						newstate.IDEX.instr =state.IDEX.instr;
						newstate.IDEX.pcplus1=state.IDEX.pcplus1;
						newstate.IDEX.readregA=state.reg[((state.IDEX.instr)>>19)&0x7];
						newstate.IDEX.readregB=state.reg[((state.IDEX.instr)>>16)&0x7];
						newstate.IDEX.offset =field2(newstate.IDEX.instr);
						
						newstate.EXMEM.instr=29360128;
						newstate.EXMEM.branchtarget=0;
						newstate.EXMEM.aluresult=0;
						newstate.EXMEM.readreg=0;	
						newstate.retired--;
						newstate.fetched--;
						newstate.pc--;
						lwaddLoadStall = 1;
					}else{
						newstate.EXMEM.aluresult=state.IDEX.readregA+state.IDEX.readregB;
						newstate.EXMEM.readreg=state.IDEX.readregA;
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;						
					}
				}else if((((state.EXMEM.instr)>>22)==3)||(((state.EXMEM.instr)>>22)==4)){
					
					if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
						newstate.EXMEM.aluresult = temp+temp;
						newstate.EXMEM.readreg=temp;	
					}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
						if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = temp+tempWBEND;
							newstate.EXMEM.readreg=temp;							
							
						}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = temp+tempWBEND;
							newstate.EXMEM.readreg=temp;
							
						}else{
							newstate.EXMEM.aluresult =temp+state.IDEX.readregB;
							newstate.EXMEM.readreg=temp;	
						}
					}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
						if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = temp+tempWBEND;
							newstate.EXMEM.readreg=tempWBEND;	
						}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = temp+tempWBEND;
							newstate.EXMEM.readreg=tempWBEND;	
						}else{						
							newstate.EXMEM.aluresult = temp+state.IDEX.readregA;
							newstate.EXMEM.readreg=state.IDEX.readregA;
						}							
					}					
				}else{
					newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
					
					if(lwaddLoadStall==1){

						if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
							newstate.EXMEM.aluresult = temp+temp;
							newstate.EXMEM.readreg=temp;	
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+tempWBEND;
								newstate.EXMEM.readreg=temp;							
								
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+tempWBEND;
								newstate.EXMEM.readreg=temp;
								
							}else{
								newstate.EXMEM.aluresult =temp+state.IDEX.readregB;
								newstate.EXMEM.readreg=temp;	
							}
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+tempWBEND;
								newstate.EXMEM.readreg=tempWBEND;	
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+tempWBEND;
								newstate.EXMEM.readreg=tempWBEND;	
							}else{						
								newstate.EXMEM.aluresult = temp+state.IDEX.readregA;
								newstate.EXMEM.readreg=state.IDEX.readregA;
							}							
						}
						lwaddLoadStall=0;
					}else{
						newstate.EXMEM.aluresult=state.IDEX.readregA+state.IDEX.readregB;
						newstate.EXMEM.readreg=state.IDEX.readregA;	
					}
										
				}
				
			}else if(((newstate.EXMEM.instr)>>22) == 1){//nand
		
				if((((state.EXMEM.instr)>>22)==1)|| (((state.EXMEM.instr)>>22)==0)){
					if((((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7))&&(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7))){

						newstate.EXMEM.aluresult=~(tempEXMEM&tempEXMEM);
						newstate.EXMEM.readreg=tempEXMEM;	
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						
					}else if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>19)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=~(temp&tempEXMEM);
							newstate.EXMEM.readreg=temp;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>19)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=~(tempWBEND&tempEXMEM);
							newstate.EXMEM.readreg=tempWBEND;	

						}else{						
						
							newstate.EXMEM.aluresult=~(state.IDEX.readregA&tempEXMEM);
							newstate.EXMEM.readreg=state.IDEX.readregA;	
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						}
					}else if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>16)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=~(temp&tempEXMEM);
							newstate.EXMEM.readreg=tempEXMEM;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>16)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=~(tempWBEND&tempEXMEM);
							newstate.EXMEM.readreg=tempEXMEM;	

						}else{					
							newstate.EXMEM.aluresult=~(tempEXMEM&state.IDEX.readregB);
							newstate.EXMEM.readreg=tempEXMEM;	
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						}
					}else{
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=~(state.IDEX.readregA&state.IDEX.readregB);
						newstate.EXMEM.readreg=state.IDEX.readregA;	
					}
					
				}else if(((state.EXMEM.instr)>>22)==2){
					if(((((newstate.EXMEM.instr)>>19)&0x7) == (((state.EXMEM.instr)>>19)&0x7)) || ((((newstate.EXMEM.instr)>>16)&0x7) == (((state.EXMEM.instr)>>19)&0x7))){
						newstate.IFID.instr=state.IFID.instr;
						newstate.IFID.pcplus1=state.IFID.pcplus1;	
						
						newstate.IDEX.instr =state.IDEX.instr;
						newstate.IDEX.pcplus1=state.IDEX.pcplus1;
						newstate.IDEX.readregA=state.reg[((state.IDEX.instr)>>19)&0x7];
						newstate.IDEX.readregB=state.reg[((state.IDEX.instr)>>16)&0x7];
						newstate.IDEX.offset =field2(newstate.IDEX.instr);	
						
						newstate.EXMEM.instr=29360128;
						newstate.EXMEM.branchtarget=0;
						newstate.EXMEM.aluresult=0;
						newstate.EXMEM.readreg=0;	
						newstate.retired--;
						newstate.fetched--;
						newstate.pc--;
						lwaddLoadStall = 1;
					}else{
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=~(state.IDEX.readregA&state.IDEX.readregB);
						newstate.EXMEM.readreg=state.IDEX.readregA;	
					}
				}else if((((state.EXMEM.instr)>>22)==3)||(((state.EXMEM.instr)>>22)==4)){
					if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
						newstate.EXMEM.aluresult = ~(temp&temp);
						newstate.EXMEM.readreg=temp;	
					}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
						if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = ~(temp&tempWBEND);
							newstate.EXMEM.readreg=temp;							
							
						}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = ~(temp&tempWBEND);
							newstate.EXMEM.readreg=temp;
							
						}else{
							newstate.EXMEM.aluresult =~(temp&state.IDEX.readregB);
							newstate.EXMEM.readreg=temp;	
						}
					}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
						if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = ~(temp&tempWBEND);
							newstate.EXMEM.readreg=tempWBEND;	
						}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
							newstate.EXMEM.aluresult = ~(temp&tempWBEND);
							newstate.EXMEM.readreg=tempWBEND;	
						}else{						
							newstate.EXMEM.aluresult = ~(temp&state.IDEX.readregA);
							newstate.EXMEM.readreg=state.IDEX.readregA;
						}							
					}	
				}else{
					newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
				
					if(lwaddLoadStall==1){
						if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
							newstate.EXMEM.aluresult = ~(temp&temp);
							newstate.EXMEM.readreg=temp;	
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = ~(temp&tempWBEND);
								newstate.EXMEM.readreg=temp;							
								
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = ~(temp&tempWBEND);
								newstate.EXMEM.readreg=temp;
								
							}else{
								newstate.EXMEM.aluresult =~(temp&state.IDEX.readregB);
								newstate.EXMEM.readreg=temp;	
							}
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = ~(temp&tempWBEND);
								newstate.EXMEM.readreg=tempWBEND;	
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = ~(temp&tempWBEND);
								newstate.EXMEM.readreg=tempWBEND;	
							}else{						
								newstate.EXMEM.aluresult = ~(temp&state.IDEX.readregA);
								newstate.EXMEM.readreg=state.IDEX.readregA;
							}							
						}
						lwaddLoadStall=0;
					}else{
						newstate.EXMEM.aluresult=~(state.IDEX.readregA&state.IDEX.readregB);
						newstate.EXMEM.readreg=state.IDEX.readregA;	
					}	
				}	
			}else if((((newstate.EXMEM.instr)>>22) == 2)||(((newstate.EXMEM.instr)>>22) == 3)){//lw sw
				if((((state.EXMEM.instr)>>22)==2)&&(((newstate.EXMEM.instr)>>22) == 2)){
					if((((newstate.EXMEM.instr)>>16)&0x7) == (((state.EXMEM.instr)>>19)&0x7)){
						newstate.IFID.instr=state.IFID.instr;
						newstate.IFID.pcplus1=state.IFID.pcplus1;	
						
						newstate.IDEX.instr =state.IDEX.instr;
						newstate.IDEX.pcplus1=state.IDEX.pcplus1;	
						
						newstate.IDEX.readregA=state.reg[((state.IDEX.instr)>>19)&0x7];
						newstate.IDEX.readregB=state.reg[((state.IDEX.instr)>>16)&0x7];
						newstate.IDEX.offset =signextend((state.IDEX.instr)&0xFFFF);
						
						newstate.EXMEM.instr=29360128;
						newstate.EXMEM.branchtarget=0;
						newstate.EXMEM.aluresult=0;
						newstate.EXMEM.readreg=0;	
						newstate.retired--;
						newstate.fetched--;
						newstate.pc--;
						lwaddLoadStall = 1;
					}else{
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;	
						newstate.EXMEM.aluresult=state.IDEX.offset+state.IDEX.readregB;
						newstate.EXMEM.readreg=state.IDEX.readregA;		
					}
				}else if(((((state.EXMEM.instr)>>22)==1)||(((state.EXMEM.instr)>>22)==0)) &&(((newstate.EXMEM.instr)>>22) == 2)){
					
					if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7)){

						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=state.IDEX.readregA+tempEXMEM;
						newstate.EXMEM.readreg=state.IDEX.readregA;	
					}else if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>16)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>16)&0x7))){
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=state.IDEX.readregA+temp;
						newstate.EXMEM.readreg=state.IDEX.readregA;						
					}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>16)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>16)&0x7))){
						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=state.IDEX.readregA+tempEXMEM;
						newstate.EXMEM.readreg=state.IDEX.readregA;							
					}							
				}else if(((((state.EXMEM.instr)>>22)==1)||(((state.EXMEM.instr)>>22)==0)||(((state.EXMEM.instr)>>22)==2)) &&(((newstate.EXMEM.instr)>>22)==3)){
					
					if(((((state.EXMEM.instr)>>22)==2)&&(((newstate.EXMEM.instr)>>22) == 3))&&(((((newstate.EXMEM.instr)>>16)&0x7) == (((state.EXMEM.instr)>>19)&0x7))||((((newstate.EXMEM.instr)>>19)&0x7) == (((state.EXMEM.instr)>>19)&0x7)))){
						if(((((newstate.EXMEM.instr)>>16)&0x7) == (((state.EXMEM.instr)>>19)&0x7))){

							newstate.IFID.instr=state.IFID.instr;
							newstate.IFID.pcplus1=state.IFID.pcplus1;	
							
							newstate.IDEX.instr =state.IDEX.instr;
							newstate.IDEX.pcplus1=state.IDEX.pcplus1;
							newstate.IDEX.readregA=state.reg[((state.IDEX.instr)>>19)&0x7];
							newstate.IDEX.readregB=state.reg[((state.IDEX.instr)>>16)&0x7];
							newstate.IDEX.offset =signextend((state.IDEX.instr)&0xFFFF);	
							
							newstate.EXMEM.instr=29360128;
							newstate.EXMEM.branchtarget=0;
							newstate.EXMEM.aluresult=0;
							newstate.EXMEM.readreg=0;	
							newstate.retired--;
							newstate.fetched--;
							newstate.pc--;
							lwaddLoadStall = 1;
						
						}else if((((newstate.EXMEM.instr)>>19)&0x7) == (((state.EXMEM.instr)>>19)&0x7)){
							lwaddLoadStall=1;
							newstate.IFID.instr=state.IFID.instr;
							newstate.IFID.pcplus1=state.IFID.pcplus1;	
							
							newstate.IDEX.instr =state.IDEX.instr;
							newstate.IDEX.pcplus1=state.IDEX.pcplus1;
							newstate.IDEX.readregA=state.reg[((state.IDEX.instr)>>19)&0x7];
							newstate.IDEX.readregB=state.reg[((state.IDEX.instr)>>16)&0x7];
							newstate.IDEX.offset =signextend((state.IDEX.instr)&0xFFFF);	
							
							newstate.EXMEM.instr=29360128;
							newstate.EXMEM.branchtarget=0;
							newstate.EXMEM.aluresult=0;
							newstate.EXMEM.readreg=0;	
							newstate.retired--;
							newstate.fetched--;
							newstate.pc--;					
						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;	
							newstate.EXMEM.aluresult=state.IDEX.offset+state.IDEX.readregB;
							newstate.EXMEM.readreg=state.IDEX.readregA;						
						}					
					}else if((((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7))&&(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7))){

						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=tempEXMEM+state.IDEX.offset;
						newstate.EXMEM.readreg=tempEXMEM;	
						
					}else if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>16)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=state.IDEX.offset+temp;
							newstate.EXMEM.readreg=tempEXMEM;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>16)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempWBEND+tempWBEND;
							newstate.EXMEM.readreg=tempEXMEM;	

						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=state.IDEX.offset+state.IDEX.readregB;
							newstate.EXMEM.readreg=tempEXMEM;
						}						
					}else if(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>19)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempEXMEM+state.IDEX.offset;
							newstate.EXMEM.readreg=temp;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>19)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=state.IDEX.offset+tempEXMEM;
							newstate.EXMEM.readreg=tempWBEND;	

						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=state.IDEX.offset+tempEXMEM;
							newstate.EXMEM.readreg=state.IDEX.readregA;
						}							
					}				
				}else{
					newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
					

					if(lwaddLoadStall==1){
						
						if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
							newstate.EXMEM.aluresult = temp+state.IDEX.offset;
							newstate.EXMEM.readreg=temp;	
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = state.IDEX.offset+ tempWBEND;
								newstate.EXMEM.readreg=temp;							
								
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = state.IDEX.offset+ tempWBEND;
								newstate.EXMEM.readreg=temp;
								
							}else{
								newstate.EXMEM.aluresult = state.IDEX.offset+ state.IDEX.readregB;
								newstate.EXMEM.readreg=temp;	
							}
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+ state.IDEX.offset;
								newstate.EXMEM.readreg=tempWBEND;	
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+ state.IDEX.offset;
								newstate.EXMEM.readreg=tempWBEND;	
							}else{						
								newstate.EXMEM.aluresult = temp+ state.IDEX.offset;
								newstate.EXMEM.readreg=state.IDEX.readregA;
							}							
						}

						lwaddLoadStall=0;
					}else{
						if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
							newstate.EXMEM.aluresult = temp+state.IDEX.offset;
							newstate.EXMEM.readreg=temp;	
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = state.IDEX.offset+ tempWBEND;
								newstate.EXMEM.readreg=temp;							
								
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = state.IDEX.offset+ tempWBEND;
								newstate.EXMEM.readreg=temp;
								
							}else{
								newstate.EXMEM.aluresult = state.IDEX.offset+ state.IDEX.readregB;
								newstate.EXMEM.readreg=temp;	
							}
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+ state.IDEX.offset;
								newstate.EXMEM.readreg=tempWBEND;	
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp+ state.IDEX.offset;
								newstate.EXMEM.readreg=tempWBEND;	
							}else{						
								newstate.EXMEM.aluresult = temp+ state.IDEX.offset;
								newstate.EXMEM.readreg=state.IDEX.readregA;
							}
							
						}else{
							newstate.EXMEM.aluresult=state.IDEX.offset+state.IDEX.readregB;
							newstate.EXMEM.readreg=state.IDEX.readregA;	
						}								
					}					
				}
			}else if(((newstate.EXMEM.instr)>>22) == 4){
				
				 if(((state.EXMEM.instr)>>22)==2){
					if(((((newstate.EXMEM.instr)>>19)&0x7) == (((state.EXMEM.instr)>>19)&0x7)) || (((((newstate.EXMEM.instr)>>16)&0x7)== (((state.EXMEM.instr)>>19)&0x7)))){
						newstate.IFID.instr=state.IFID.instr;
						newstate.IFID.pcplus1=state.IFID.pcplus1;	
						
						newstate.IDEX.instr =state.IDEX.instr;
						newstate.IDEX.pcplus1=state.IDEX.pcplus1;
						newstate.IDEX.readregA=state.reg[((state.IDEX.instr)>>19)&0x7];
						newstate.IDEX.readregB=state.reg[((state.IDEX.instr)>>16)&0x7];
						newstate.IDEX.offset =signextend((state.IDEX.instr)&0xFFFF);	
						
						newstate.EXMEM.instr=29360128;
						newstate.EXMEM.branchtarget=0;
						newstate.EXMEM.aluresult=0;
						newstate.EXMEM.readreg=0;	
						newstate.retired--;
						newstate.fetched--;
						newstate.pc--;
						lwaddLoadStall = 1;
					}
				}else if(((((state.EXMEM.instr)>>22)==1)||(((state.EXMEM.instr)>>22)==0))){
					
					if((((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>19)&0x7))&&(((state.EXMEM.instr)&0x7)==(((newstate.EXMEM.instr)>>16)&0x7))){

						newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
						newstate.EXMEM.aluresult=tempEXMEM-tempEXMEM;
						newstate.EXMEM.readreg=tempEXMEM;	
					}else if(((newstate.EXMEM.instr>>19)&0x7)==(((state.EXMEM.instr))&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>16)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=temp-tempEXMEM;
							newstate.EXMEM.readreg=tempEXMEM;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>16)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>16)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempWBEND-tempEXMEM;
							newstate.EXMEM.readreg=tempEXMEM;	

						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempEXMEM-state.IDEX.readregB;
							newstate.EXMEM.readreg=tempEXMEM;	
						}						
					}else if(((newstate.EXMEM.instr>>16)&0x7)==(((state.EXMEM.instr))&0x7)){
						if(((memwbOP==0||memwbOP==1)&&(memwboff==((newstate.EXMEM.instr)>>19)&0x7))||((memwbOP==2)&&(memwbA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=temp-tempEXMEM;
							newstate.EXMEM.readreg=temp;	
						
						}else if(((wbendOP==0||wbendOP==1)&&(wbendoff==((newstate.EXMEM.instr)>>19)&0x7))||((wbendOP==2)&&(wbendA==((newstate.EXMEM.instr)>>19)&0x7))){
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempWBEND-tempEXMEM;
							newstate.EXMEM.readreg=tempWBEND;	

						}else{
							newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
							newstate.EXMEM.aluresult=tempEXMEM-state.IDEX.readregA;
							newstate.EXMEM.readreg=tempEXMEM;	
						}						
					}						
					
				}else{				
					newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
					
					if(lwaddLoadStall==1){
						if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
							newstate.EXMEM.aluresult = temp-temp;
							newstate.EXMEM.readreg=temp;	
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=temp;							
								
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=temp;
								
							}else{
								newstate.EXMEM.aluresult =temp-state.IDEX.readregB;
								newstate.EXMEM.readreg=temp;	
							}
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=tempWBEND;	
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=tempWBEND;	
							}else{						
								newstate.EXMEM.aluresult = temp-state.IDEX.readregA;
								newstate.EXMEM.readreg=state.IDEX.readregA;
							}							
						}
						lwaddLoadStall=0;
					}else{
						if(((((state.IDEX.instr)>>19)&0x7) == (((state.MEMWB.instr)>>19)&0x7)) && ((((state.IDEX.instr)>>16)&0x7)== (((state.MEMWB.instr)>>19)&0x7))){
							newstate.EXMEM.aluresult = temp-temp;
							newstate.EXMEM.readreg=temp;	
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>16)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=temp;							
								
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=temp;
								
							}else{
								newstate.EXMEM.aluresult =temp-state.IDEX.readregB;
								newstate.EXMEM.readreg=temp;	
							}
						}else if(((opcode(state.MEMWB.instr)==2))&&((((state.IDEX.instr)>>16)&0x7) == (field0(state.MEMWB.instr)))){
							if(((opcode(tempinstrWBEND)==0)||(opcode(tempinstrWBEND)==1))&&((((state.IDEX.instr)>>19)&0x7) == (field2(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=tempWBEND;	
							}else if(((opcode(tempinstrWBEND)==2))&&((((state.IDEX.instr)>>19)&0x7) == (field0(tempinstrWBEND)))){
								newstate.EXMEM.aluresult = temp-tempWBEND;
								newstate.EXMEM.readreg=tempWBEND;	
							}else{						
								newstate.EXMEM.aluresult = temp-state.IDEX.readregA;
								newstate.EXMEM.readreg=state.IDEX.readregA;
							}							
						}else{
							newstate.EXMEM.aluresult=state.IDEX.readregA+state.IDEX.readregB;
							newstate.EXMEM.readreg=state.IDEX.readregA;
						}
					}					
				}					
			}else{//noop halt
				newstate.EXMEM.branchtarget=state.IDEX.offset+state.IDEX.pcplus1;
				newstate.EXMEM.aluresult=0;
				newstate.EXMEM.readreg=state.IDEX.readregA;				
				
			}
			
			tempEXMEM = newstate.EXMEM.aluresult;
			/*------------------ MEM stage ----------------- */
			newstate.MEMWB.instr =state.EXMEM.instr;


			if((((newstate.MEMWB.instr)>>22) == 0)||(((newstate.MEMWB.instr)>>22) == 1)){
				newstate.MEMWB.writedata=state.EXMEM.aluresult;
				newstate.pc++;
			}else if(((newstate.MEMWB.instr)>>22) == 2){//lw
				
				
				newstate.MEMWB.writedata=state.datamem[state.EXMEM.aluresult];

				newstate.pc++;
				

			}else if(((newstate.MEMWB.instr)>>22) == 3){//sw

				newstate.datamem[state.EXMEM.aluresult]=state.EXMEM.readreg;
				newstate.MEMWB.writedata=state.EXMEM.readreg;
				newstate.pc++;
			}else if(((newstate.MEMWB.instr)>>22) == 4){
				newstate.branches++;
				if(state.EXMEM.aluresult==0){
					if(opcode(newstate.EXMEM.instr)==6){

						newstate.retired = newstate.retired-1;	
					}else if(opcode(newstate.IDEX.instr)==6){

						newstate.retired = newstate.retired-2;	
					}else {
						newstate.retired = newstate.retired-3;
					}
					newstate.mispreds++;
					newstate.pc = state.EXMEM.branchtarget;
					
					newstate.IFID.instr=29360128;
					newstate.IFID.pcplus1=0;	
					
					newstate.IDEX.instr =29360128;
					newstate.IDEX.pcplus1=0;
					newstate.IDEX.readregA=0;
					newstate.IDEX.readregB=0;
					newstate.IDEX.offset = 0;	
					
					newstate.EXMEM.instr=29360128;
					newstate.EXMEM.branchtarget=0;
					newstate.EXMEM.aluresult=0;
					newstate.EXMEM.readreg=0;		
					
				}else{
					newstate.pc++;
				}
			}else if(newstate.MEMWB.instr==29360128){
				newstate.MEMWB.writedata=0;
				newstate.pc++;				
			}else{
				newstate.MEMWB.writedata=0;
				newstate.pc++;
			}
			temp = newstate.MEMWB.writedata;
			tempinstrMEMWB = newstate.MEMWB.instr;
			newstate.retired++;
			
			/*------------------ WB stage ----------------- */
			newstate.WBEND.instr =state.MEMWB.instr;
			if((((newstate.WBEND.instr)>>22) == 0)||(((newstate.WBEND.instr)>>22) == 1)){
				newstate.reg[(state.MEMWB.instr)&0x7] = state.MEMWB.writedata;
				newstate.WBEND.writedata=state.MEMWB.writedata;
			}else if(((newstate.WBEND.instr)>>22) == 2){
				newstate.reg[((state.MEMWB.instr)>>19)&0x7] = state.MEMWB.writedata;
				newstate.WBEND.writedata=state.MEMWB.writedata;
				tempinstrWBEND = newstate.WBEND.instr;
				tempWBEND = newstate.WBEND.writedata;
			}else{
				newstate.WBEND.writedata=state.MEMWB.writedata;
			}
			
			state = newstate;
			
			
		}

	}
	return 0;
}
