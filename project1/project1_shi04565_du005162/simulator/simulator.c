#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct state_struct {
	int pc;
	int mem[51];
	int reg[8];
	int num_memory;
} statetype;

int power(int a){
	int result = 1;
	for(int i=0;i<a;i++){
		result = result*2;
	}
	return result;
}

int btod(char a[]){
	int dec = 0;
	if(strlen(a)==16){//offset
		if(a[0]=='1'){//negative 
			int temp=0;
			for(int i=1; i<strlen(a);i++){
				if(a[i]=='1'){
					temp = temp+power(15-i);
				}else {
					temp = temp+0;
				}
			}
			dec = 0-(32768-temp);
		}else{//positive
			int temp=0;
			for(int i=0; i<strlen(a);i++){
				if(a[i]=='1'){
					temp = temp+power(15-i);
				}else {
					temp = temp+0;
				}
			}
			dec=temp;
		}
	}else{
		for(int i=0; i<strlen(a);i++){

			if(a[i]=='1'){
				dec = dec+power(2-i);
			}else {
				dec = dec+0;
			}
		}
	}
	return dec;
}

int convert_num(int num){
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return(num);
}

void print_stats(int n_instrs){
	printf("INSTRUCTIONS: %d\n", n_instrs); // total executed instructions
}

void print_state(statetype *stateptr){
	int i;
	printf("\n@@@\nstate:\n");
	printf("\tpc %d\n", stateptr->pc);
	printf("\tmemory:\n");
	
	for(i = 0; i < stateptr->num_memory; i++){
		printf("\t\tmem[%d]=%d\n", i, stateptr->mem[i]);
	}
	
	printf("\tregisters:\n");
	
	for(i = 0; i < 8; i++){
		printf("\t\treg[%d]=%d\n", i, stateptr->reg[i]);
	}
	
	printf("end state\n");
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
	char bina[51][33];
	char deci[51][12];
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
		statetype stateptr;
		stateptr.num_memory=j;
		memset(stateptr.mem,0,sizeof(stateptr.mem));
		memset(stateptr.reg,0,sizeof(stateptr.reg));
		for(int i=0;i<j;i++){
			stateptr.mem[i]=atoi(deci[i]);
		}
		stateptr.pc=0;
		
		int counter = 1;
		int check=0;
		int haltLine = 0;
		
		print_state(&stateptr);
		
		while(strcmp(deci[haltLine],"25165824\n")!=0){
			haltLine++;
			if(haltLine==j){
				printf("ERROR: <Halt instruction needed>\n");
				exit(-1);
			}
		}

		while(1==1){
			char op[4] = {bina[stateptr.pc][7],bina[stateptr.pc][8],bina[stateptr.pc][9]};
			char commonUnused[8] = {bina[stateptr.pc][0],bina[stateptr.pc][1],bina[stateptr.pc][2],
				bina[stateptr.pc][3],bina[stateptr.pc][4],bina[stateptr.pc][5],bina[stateptr.pc][6]};
			if(strcmp(commonUnused,"0000000")!=0){
				printf("ERROR: <Common unused bits must be all zeros>\n");
				exit(-1);
			}
			if(strcmp(op,"000")==0){//add
			
				char ra[4]={bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12]};
				char rb[4]={bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15]};
				char destr[4]={bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				char rTypeUnused[14] = {bina[stateptr.pc][16],bina[stateptr.pc][17],bina[stateptr.pc][18],
					bina[stateptr.pc][19],bina[stateptr.pc][20],bina[stateptr.pc][21],bina[stateptr.pc][22],
					bina[stateptr.pc][23],bina[stateptr.pc][24],bina[stateptr.pc][25],bina[stateptr.pc][26],
					bina[stateptr.pc][27],bina[stateptr.pc][28]};
				if(strcmp(rTypeUnused,"0000000000000")!=0){
					printf("ERROR: <R-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;
				
				int a= btod(ra); 
				int b=btod(rb);
				int dest=btod(destr);
				if(dest==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				stateptr.reg[dest] = stateptr.reg[a]+stateptr.reg[b];
				
			}else if(strcmp(op,"001")==0){//nand
							
				char ra[4]={bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12]};
				char rb[4]={bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15]};
				char destr[4]={bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				char rTypeUnused[14] = {bina[stateptr.pc][16],bina[stateptr.pc][17],bina[stateptr.pc][18],
					bina[stateptr.pc][19],bina[stateptr.pc][20],bina[stateptr.pc][21],bina[stateptr.pc][22],
					bina[stateptr.pc][23],bina[stateptr.pc][24],bina[stateptr.pc][25],bina[stateptr.pc][26],
					bina[stateptr.pc][27],bina[stateptr.pc][28]};
				if(strcmp(rTypeUnused,"0000000000000")!=0){
					printf("ERROR: <R-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;
				
				int a= btod(ra); 
				int b=btod(rb);
				int dest=btod(destr);
				if(dest==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				stateptr.reg[dest] = ~(stateptr.reg[a]&stateptr.reg[b]);
				
			}else if(strcmp(op,"010")==0){//lw
							
				char ra[4]={bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12]};
				char rb[4]={bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15]};
				char offset[17]={bina[stateptr.pc][16],bina[stateptr.pc][17],bina[stateptr.pc][18],
					bina[stateptr.pc][19],bina[stateptr.pc][20],bina[stateptr.pc][21],
					bina[stateptr.pc][22],bina[stateptr.pc][23],bina[stateptr.pc][24],
					bina[stateptr.pc][25],bina[stateptr.pc][26],bina[stateptr.pc][27],
					bina[stateptr.pc][28],bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				stateptr.pc = stateptr.pc+1;	
				
				int a= btod(ra); 
				int b=btod(rb);
				int off=btod(offset);
				
				if(a==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				
				
				stateptr.reg[a] = stateptr.mem[stateptr.reg[b]+off];
				
					
			}else if(strcmp(op,"011")==0){//sw
							
				char ra[4]={bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12]};
				char rb[4]={bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15]};
				char offset[17]={bina[stateptr.pc][16],bina[stateptr.pc][17],bina[stateptr.pc][18],
					bina[stateptr.pc][19],bina[stateptr.pc][20],bina[stateptr.pc][21],
					bina[stateptr.pc][22],bina[stateptr.pc][23],bina[stateptr.pc][24],
					bina[stateptr.pc][25],bina[stateptr.pc][26],bina[stateptr.pc][27],
					bina[stateptr.pc][28],bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				stateptr.pc = stateptr.pc+1;
				
				int a= btod(ra);
				int b=btod(rb);
				int off=btod(offset);
				if(stateptr.reg[b]+off<=haltLine){
					printf("ERROR: <Cannot save to the memory address above 'halt' instruction>\n");
					exit(-1);
				}else{
					stateptr.mem[stateptr.reg[b]+off] = stateptr.reg[a];
				}
				
			}else if(strcmp(op,"100")==0){//beq	
				
				char ra[4]={bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12]};
				char rb[4]={bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15]};
				char offset[17]={bina[stateptr.pc][16],bina[stateptr.pc][17],bina[stateptr.pc][18],
					bina[stateptr.pc][19],bina[stateptr.pc][20],bina[stateptr.pc][21],
					bina[stateptr.pc][22],bina[stateptr.pc][23],bina[stateptr.pc][24],
					bina[stateptr.pc][25],bina[stateptr.pc][26],bina[stateptr.pc][27],
					bina[stateptr.pc][28],bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				stateptr.pc = stateptr.pc+1;
				
				int a= btod(ra); 
				int b=btod(rb);
				int off=btod(offset);
				
				if(off+stateptr.pc>haltLine){
					printf("ERROR: <Cannot jump to the memory address under 'halt' instruction>\n");
					exit(-1);
				}
				
				if(stateptr.reg[b]==stateptr.reg[a]){
					stateptr.pc = stateptr.pc+off;
				}
				
			}else if(strcmp(op,"101")==0){//jalr
				char ra[4]={bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12]};
				char rb[4]={bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15]};
				char jTypeUnused[17] = {bina[stateptr.pc][16],bina[stateptr.pc][17],bina[stateptr.pc][18],
					bina[stateptr.pc][19],bina[stateptr.pc][20],bina[stateptr.pc][21],bina[stateptr.pc][22],
					bina[stateptr.pc][23],bina[stateptr.pc][24],bina[stateptr.pc][25],bina[stateptr.pc][26],
					bina[stateptr.pc][27],bina[stateptr.pc][28],bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				if(strcmp(jTypeUnused,"0000000000000000")!=0){
					printf("ERROR: <J-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;
				int a = btod(ra); 
				int b = btod(rb);
				
				if(a==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				
				if(stateptr.reg[b]>haltLine){
					printf("ERROR: <Cannot jump to the memory address under 'halt' instruction>\n");
					exit(-1);
				}
				
				stateptr.reg[a] = stateptr.pc;
				stateptr.pc  = stateptr.reg[b];
				
			}else if(strcmp(op,"110")==0){//halt
				char oTypeUnused[23] = {bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12],
					bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15],bina[stateptr.pc][16],
					bina[stateptr.pc][17],bina[stateptr.pc][18],bina[stateptr.pc][19],bina[stateptr.pc][20],
					bina[stateptr.pc][21],bina[stateptr.pc][22],bina[stateptr.pc][23],bina[stateptr.pc][24],
					bina[stateptr.pc][25],bina[stateptr.pc][26],bina[stateptr.pc][27],bina[stateptr.pc][28],
					bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				if(strcmp(oTypeUnused,"0000000000000000000000")!=0){
					printf("ERROR: <O-type unused bits must be all zeros>\n");
					exit(-1);
				}
				break;
			}else if(strcmp(op,"111")==0){//noop
				char oTypeUnused[23] = {bina[stateptr.pc][10],bina[stateptr.pc][11],bina[stateptr.pc][12],
					bina[stateptr.pc][13],bina[stateptr.pc][14],bina[stateptr.pc][15],bina[stateptr.pc][16],
					bina[stateptr.pc][17],bina[stateptr.pc][18],bina[stateptr.pc][19],bina[stateptr.pc][20],
					bina[stateptr.pc][21],bina[stateptr.pc][22],bina[stateptr.pc][23],bina[stateptr.pc][24],
					bina[stateptr.pc][25],bina[stateptr.pc][26],bina[stateptr.pc][27],bina[stateptr.pc][28],
					bina[stateptr.pc][29],bina[stateptr.pc][30],bina[stateptr.pc][31]};
				if(strcmp(oTypeUnused,"0000000000000000000000")!=0){
					printf("ERROR: <O-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;
			}else{
				printf("ERROR: <Undefined opcode>\n");
				exit(-1);
			}
			counter++;
			print_state(&stateptr);
		}
		printf("machine halted\n");
		print_stats(counter);
	}
	return 0;
}

