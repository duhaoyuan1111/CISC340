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


typedef struct cache_struct {
	int valid;
	int dirty;
	int tag;
	int lru;
	int *block;
} cachetype;


//helper function
void lruIncrement(int assoc_number, cachetype *cache){
	for(int i=0;i<assoc_number;i++){
		if(cache[i].valid ==1){
			cache[i].lru++;
		}
	}
}


int etaCalc(int block_number){
	int eta=0;

	int temp5 = block_number;
	while(temp5>1){
		temp5 = temp5/2;
		eta++;
	}	
	return eta;
}

int saiCalc(int sets_number){
	int sai=0;
	int temp6 = sets_number;
	while(temp6>1){
		temp6 = temp6/2;
		sai++;
	}
	return sai;	
}

int indexOfKick(int assoc_number, cachetype *cache){
	int kickoff=0;
	for(int i=1;i<assoc_number;i++){
		if(cache[i].lru>cache[kickoff].lru){
			kickoff = i;
		}
	}
	return kickoff;
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

char *trimwhitespace(char *str){
  char *end;
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0) 
    return str;

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  end[1] = '\0';

  return str;
}

enum action_type {cache_to_processor, processor_to_cache, memory_to_cache, cache_to_memory,
cache_to_nowhere};

void print_action(int address, int size, enum action_type type)
{
	printf("transferring word [%i-%i] ", address, address + size - 1);
	if (type == cache_to_processor) {
		printf("from the cache to the processor\n");
	} else if (type == processor_to_cache) {
		printf("from the processor to the cache\n");
	} else if (type == memory_to_cache) {
		printf("from the memory to the cache\n");
	} else if (type == cache_to_memory) {
		printf("from the cache to the memory\n");
	} else if (type == cache_to_nowhere) {
		printf("from the cache to nowhere\n");
	}
}
	 
int main (int argc, char **argv){
	
	
FILE* mc;
char read[500];
char *fvalue = NULL;
char *bvalue = NULL;
char *svalue = NULL;
char *avalue = NULL;

int a;
while ((a = getopt (argc, argv, "f:b:s:a:")) != -1)
	switch (a)
		{
		case 'f': 
			fvalue=optarg;
			break;
		case 'b': 
			bvalue=optarg;
			break;
		case 's': 
			svalue=optarg;
			break;
		case 'a': 
			avalue=optarg;
			break;
		default:
			abort ();
		}
	
	int c, k;
	int j=0;
	
	char deci[51][50];
	for(int i=0;i<50;i++){
		memset(deci[i],'\0',sizeof(deci[i]));
	}
	
	char ff[30];
	memset(ff,'\0',sizeof(ff));
	char bb[11];
	memset(bb,'\0',sizeof(bb));
	char ss[11];
	memset(ss,'\0',sizeof(ss));
	char aa[11];
	memset(aa,'\0',sizeof(aa));
	if(fvalue==NULL){
		printf("Enter the machine code program to simulate: ");
		fgets(ff,25,stdin);
		fvalue=trimwhitespace(ff);
		printf("\n");
	}
	int block_number;
	if(bvalue==NULL){
		printf("Enter the block size of the cache (in words): ");
		fgets(bb,10,stdin);
		block_number = atoi(bb);
		int test_block=block_number;
		printf("\n");	
		if(test_block<1){
			printf("block_number should greater than zero");
			exit(-1);
		}else{
			int check=0;
			for(int i=0; i<9;i++){
				if(((test_block>>i)&1)==1){
					check++;
				}
			}
			if(check>1){
				printf("block_number should be the power of two");
				exit(-1);			
			}
		}
	
	}else{
		block_number = atoi(bvalue);
		int test_block=block_number;
		if(test_block<1){
			printf("block_number should greater than zero");
			exit(-1);
		}else{
			int check=0;
			for(int i=0; i<9;i++){
				if(((test_block>>i)&1)==1){
					check++;
				}
			}
			if(check>1){
				printf("block_number should be the power of two");
				exit(-1);			
			}
		}

	}
		
	
	int sets_number;
	if(svalue==NULL){
		printf("Enter the number of sets in the cache: ");
		fgets(ss,10,stdin);
		sets_number = atoi(ss);
		printf("\n");
		int test_set=sets_number;
		if(test_set<1){
			printf("sets_number should greater than zero");
			exit(-1);
		}else{
			int check=0;
			for(int i=0; i<9;i++){
				if(((test_set>>i)&1)==1){
					check++;
				}
			}
			if(check>1){
				printf("sets_number should be the power of two");
				exit(-1);			
			}
		}
	}else{
		sets_number= atoi(svalue);
		int test_set=sets_number;
		if(test_set<1){
			printf("sets_number should greater than zero");
			exit(-1);
		}else{
			int check=0;
			for(int i=0; i<9;i++){
				if(((test_set>>i)&1)==1){
					check++;
				}
			}
			if(check>1){
				printf("sets_number should be the power of two");
				exit(-1);			
			}
		}
	}
		
	int assoc_number;
	if(avalue==NULL){
		printf("Enter the associativity of the cache: ");
		fgets(aa,10,stdin);
		assoc_number = atoi(aa);
		printf("\n");
		int test_assoc = assoc_number;
		if(test_assoc<1){
			printf("assoc_number should greater than zero");
			exit(-1);
		}else{
			int check=0;
			for(int i=0; i<9;i++){
				if(((test_assoc>>i)&1)==1){
					check++;
				}
			}
			if(check>1){
				printf("assoc_number should be the power of two");
				exit(-1);			
			}
		}	
	}else{
		assoc_number= atoi(avalue);
		int test_assoc = assoc_number;
		if(test_assoc<1){
			printf("assoc_number should greater than zero");
			exit(-1);
		}else{
			int check=0;
			for(int i=0; i<9;i++){
				if(((test_assoc>>i)&1)==1){
					check++;
				}
			}
			if(check>1){
				printf("assoc_number should be the power of two");
				exit(-1);			
			}
		}	
	}
	
	if((assoc_number*sets_number*block_number)>256){
		printf("the product of all inputs should less than or equal to 256");
		exit(-1);			
	}
	
	cachetype** cache = (cachetype**)malloc(sets_number*sizeof(cachetype*)); 
	for(int i = 0; i < sets_number; ++i){
		cache[i] = (cachetype*)malloc(assoc_number*sizeof(cachetype)); // allocs 1 row of the second dimension
	}
	
	
	for(int i=0;i<sets_number;i++){
		
		for(int j=0;j<assoc_number;j++){
			cache[i][j].valid=0;
			cache[i][j].dirty=0;
			cache[i][j].tag = 0;
			cache[i][j].lru = 0;
			cache[i][j].block =(int*)malloc(block_number*sizeof(int));
			memset(cache[i][j].block,0,block_number);
		}
	}

	if(fvalue !=NULL){	

		mc=fopen(fvalue,"r");

		while(fgets(read,sizeof(read),mc)!= NULL){
			
			for(int i=0; i<12;i++){
				deci[j][i]=read[i];
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
		
		int check=0;
		int haltLine = 0;
	
		
		int eta=etaCalc(block_number);
		int sai=saiCalc(sets_number);
		
		while(1==1){
			int set_clear=(stateptr.pc>>eta)&((1<<sai)-1);
			int block_clear = (stateptr.pc)&((1<<eta)-1);
			int tag_clear = (stateptr.pc)>>(eta+sai);
			
			int dataFromcache;
			int check = 0;
			int temp=0;
			for(int i=0;i<assoc_number;i++){
				if(cache[set_clear][i].valid ==1){
					if(cache[set_clear][i].tag == tag_clear){
						dataFromcache = cache[set_clear][i].block[block_clear];
						cache[set_clear][i].lru = 0;
						check =1;				
						print_action(stateptr.pc, 1,cache_to_processor);
					}else{
						temp++;
					}
				}
			}

			if(check!=0){
				lruIncrement(assoc_number,cache[set_clear]);
			}
	
			if(check==0){
				if(temp==assoc_number){
					int kickoff;
					if(assoc_number==1){
						kickoff=0;
					}else{				
						kickoff=indexOfKick(assoc_number,cache[set_clear]);
					}
					if(cache[set_clear][kickoff].dirty==0){
						int temp2 = (cache[set_clear][kickoff].tag<<(eta+sai))+(set_clear<<(eta));
						print_action(temp2, block_number,cache_to_nowhere);
						for(int i=0;i<block_number;i++){
							cache[set_clear][kickoff].block[i]=stateptr.mem[stateptr.pc+i];
						}
						dataFromcache = cache[set_clear][kickoff].block[block_clear];
						cache[set_clear][kickoff].valid=1;
						cache[set_clear][kickoff].tag=tag_clear;
						cache[set_clear][kickoff].dirty=0;
						cache[set_clear][kickoff].lru=0;
						
						lruIncrement(assoc_number,cache[set_clear]);
						int move = (stateptr.pc>>eta)<<eta;					
						print_action(move, block_number,memory_to_cache);
					}else{
						int temp2 = (cache[set_clear][kickoff].tag<<(eta+sai))+(set_clear<<(eta));
						for(int i=0;i<block_number;i++){
								stateptr.mem[temp2+i]=cache[set_clear][kickoff].block[i];
						}
						print_action(temp2, block_number,cache_to_memory);	
						for(int i=0;i<block_number;i++){
								cache[set_clear][kickoff].block[i]=stateptr.mem[stateptr.pc+i];
						}
						dataFromcache = cache[set_clear][kickoff].block[block_clear];
						cache[set_clear][kickoff].valid=1;
						cache[set_clear][kickoff].tag=tag_clear;
						cache[set_clear][kickoff].dirty=0;
						cache[set_clear][kickoff].lru=0;
						lruIncrement(assoc_number,cache[set_clear]);
						int move = (stateptr.pc>>eta)<<eta;						
						print_action(move, block_number,memory_to_cache);
					} 
						
				}else{
					int counter = 0;
					int move = (stateptr.pc>>eta)<<eta;	
					while(1==1){
						if(cache[set_clear][counter].valid==0){
							for(int i=0;i<block_number;i++){
									cache[set_clear][counter].block[i]=stateptr.mem[move+i];
							}
							dataFromcache = cache[set_clear][counter].block[block_clear];
							cache[set_clear][counter].valid=1;
							cache[set_clear][counter].tag=tag_clear;
							cache[set_clear][counter].dirty=0;
							cache[set_clear][counter].lru=0;
							lruIncrement(assoc_number,cache[set_clear]);	
							int temp2 = (cache[set_clear][counter].tag<<(eta+sai))+(set_clear<<(eta));
							print_action(temp2, block_number,memory_to_cache);	
							break;					
						}					
						counter++;					
					}
				}
				
				print_action(stateptr.pc, 1,cache_to_processor);
				
			}

		
			int commonUnused = (dataFromcache>>25);
			if(commonUnused!=0){
				printf("ERROR: <Common unused bits must be all zeros>\n");
				exit(-1);
			}
			int op = opcode(dataFromcache);
			int ra = field0(dataFromcache);
			int rb = field1(dataFromcache);
			int offDest= field2(dataFromcache);
			
			if(op==0){//add
			
				int rTypeUnused= (dataFromcache>>3)&8191;
				if(rTypeUnused!=0){
					printf("ERROR: <R-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;
				if(offDest==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				stateptr.reg[offDest] = stateptr.reg[ra]+stateptr.reg[rb];
				
			}else if(op==1){//nand
							
				int rTypeUnused= (dataFromcache>>3)&8191;
				if(rTypeUnused!=0){
					printf("ERROR: <R-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;

				if(offDest==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				stateptr.reg[offDest] = ~(stateptr.reg[ra]&stateptr.reg[rb]);
				
			}else if(op==2){//lw
						
				stateptr.pc = stateptr.pc+1;	
				
				if(stateptr.reg[ra]!=0 && ra==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				int memaddress = stateptr.reg[rb]+offDest;
				int set_clear=(memaddress>>eta)&((1<<sai)-1);
				int block_clear = (memaddress)&((1<<eta)-1);
				int tag_clear = (memaddress)>>(eta+sai);
				
				int dataFromcache;
				int check=0;
				int temp=0;
				for(int i=0;i<assoc_number;i++){
					if(cache[set_clear][i].valid ==1){
						if(cache[set_clear][i].tag == tag_clear){
							dataFromcache = cache[set_clear][i].block[block_clear];
							cache[set_clear][i].lru = 0;
							check=1;
							print_action(memaddress, 1,cache_to_processor);
						}else{
							temp++;
						}
					}
				}
				
				if(check!=0){
					lruIncrement(assoc_number,cache[set_clear]);
					
				}
				if(check==0){
					
					if(temp==assoc_number){
						int kickoff;
						if(assoc_number==1){
							kickoff=0;
						}else{				
							kickoff=indexOfKick(assoc_number,cache[set_clear]);
						}
						
						if(cache[set_clear][kickoff].dirty==0){
							int temp2 = (cache[set_clear][kickoff].tag<<(eta+sai))+(set_clear<<(eta));
							print_action(temp2, block_number,cache_to_nowhere);
							for(int i=0;i<block_number;i++){
								cache[set_clear][kickoff].block[i]=stateptr.mem[memaddress+i];
							}
							dataFromcache = cache[set_clear][kickoff].block[block_clear];
							cache[set_clear][kickoff].valid=1;
							cache[set_clear][kickoff].tag=tag_clear;
							cache[set_clear][kickoff].dirty=0;
							cache[set_clear][kickoff].lru=0;
							
							lruIncrement(assoc_number,cache[set_clear]);
							int move = (memaddress>>eta)<<eta;					
							print_action(move, block_number,memory_to_cache);
						}else{
							int temp2 = (cache[set_clear][kickoff].tag<<(eta+sai))+(set_clear<<(eta));
						
							for(int i=0;i<block_number;i++){
									stateptr.mem[temp2+i]=cache[set_clear][kickoff].block[i];
							}
							print_action(temp2, block_number,cache_to_memory);	
							for(int i=0;i<block_number;i++){
									cache[set_clear][kickoff].block[i]=stateptr.mem[memaddress+i];
							}
							dataFromcache = cache[set_clear][kickoff].block[block_clear];
							cache[set_clear][kickoff].valid=1;
							cache[set_clear][kickoff].tag=tag_clear;
							cache[set_clear][kickoff].dirty=0;
							cache[set_clear][kickoff].lru=0;
							lruIncrement(assoc_number,cache[set_clear]);
							int move = (memaddress>>eta)<<eta;
							print_action(move, block_number,memory_to_cache);
						} 
							
					}else{
						int counter = 0;
						int move = (memaddress>>eta)<<eta;
						while(1==1){
							int aa =sizeof(cache[set_clear][counter].block);
							int bb =sizeof(cache[set_clear][counter]);
							int cc =sizeof(cache[set_clear]);
							if(cache[set_clear][counter].valid==0){
								for(int i=0;i<block_number;i++){
									cache[set_clear][counter].block[i]=stateptr.mem[move+i];
								}
								dataFromcache = cache[set_clear][counter].block[block_clear];
								cache[set_clear][counter].valid=1;
								cache[set_clear][counter].tag=tag_clear;
								cache[set_clear][counter].dirty=0;
								cache[set_clear][counter].lru=0;
								lruIncrement(assoc_number,cache[set_clear]);
								int temp2 = (cache[set_clear][counter].tag<<(eta+sai))+(set_clear<<(eta));								
								print_action(temp2, block_number,memory_to_cache);	
								break;					
							}					
							counter++;					
						}
					}
					print_action(memaddress, 1,cache_to_processor);
					
				}
				stateptr.reg[ra] = dataFromcache;

					
			}else if(op==3){//sw

				stateptr.pc = stateptr.pc+1;
				
				if(stateptr.reg[rb]+offDest<=haltLine){
					printf("ERROR: <Cannot save to the memory address above 'halt' instruction>\n");
					exit(-1);
				}else{
				
					int memaddress = stateptr.reg[rb]+offDest;
					int set_clear=(memaddress>>eta)&((1<<sai)-1);
					int block_clear = (memaddress)&((1<<eta)-1);
					int tag_clear = (memaddress)>>(eta+sai);
					
					int dataTocache;
					int check=0;
					int temp=0;
					for(int i=0;i<assoc_number;i++){
						if(cache[set_clear][i].valid ==1){
							if(cache[set_clear][i].tag == tag_clear){
								cache[set_clear][i].block[block_clear]=stateptr.reg[ra];
								dataTocache = stateptr.reg[ra];
								check=1;
								cache[set_clear][i].dirty=1;
								cache[set_clear][i].lru = 0;
								print_action(memaddress, 1,processor_to_cache);
							}else{
								temp++;
							}
						}
					}
					if(check!=0){
						lruIncrement(assoc_number,cache[set_clear]);
						
					}
					if(check==0){
						if(temp==assoc_number){
							int kickoff;
							if(assoc_number==1){
								kickoff=0;
							}else{				
								kickoff=indexOfKick(assoc_number,cache[set_clear]);
							}
							if(cache[set_clear][kickoff].dirty==0){
								int temp2 = (cache[set_clear][kickoff].tag<<(eta+sai))+(set_clear<<(eta));
								print_action(temp2, block_number,cache_to_nowhere);
								for(int i=0;i<block_number;i++){
									cache[set_clear][kickoff].block[i]=stateptr.mem[memaddress+i];
								}
								cache[set_clear][kickoff].block[block_clear]=stateptr.reg[a];
								cache[set_clear][kickoff].valid=1;
								cache[set_clear][kickoff].tag=tag_clear;
								cache[set_clear][kickoff].dirty=1;
								cache[set_clear][kickoff].lru=0;
								
								int move = (memaddress>>eta)<<eta;	
								print_action(move, block_number,memory_to_cache);
								lruIncrement(assoc_number,cache[set_clear]);
													
								print_action(memaddress, 1,processor_to_cache);
							}else{
								int temp2 = (cache[set_clear][kickoff].tag<<(eta+sai))+(set_clear<<(eta));
								for(int i=0;i<block_number;i++){
									stateptr.mem[temp2+i]=cache[set_clear][kickoff].block[i];
								}
								print_action(temp2, block_number,cache_to_memory);	
								for(int i=0;i<block_number;i++){
									cache[set_clear][kickoff].block[i]=stateptr.mem[memaddress+i];
								}
								int move = (memaddress>>eta)<<eta;	
								print_action(move, block_number,memory_to_cache);
								cache[set_clear][kickoff].block[block_clear]=stateptr.reg[a];
								cache[set_clear][kickoff].valid=1;
								cache[set_clear][kickoff].tag=tag_clear;
								cache[set_clear][kickoff].dirty=1;
								cache[set_clear][kickoff].lru=0;
								lruIncrement(assoc_number,cache[set_clear]);				
								print_action(memaddress, 1,processor_to_cache);
							} 
								
						}else{
							int counter = 0;
							int temp2 = ((memaddress>>eta)<<eta);
							while(1==1){
								if(cache[set_clear][counter].valid==0){
									for(int i=0;i<block_number;i++){
											cache[set_clear][counter].block[i]=stateptr.mem[memaddress+i];
									}
									print_action(temp2, block_number,memory_to_cache);
									cache[set_clear][counter].block[block_clear]=stateptr.reg[a];
									cache[set_clear][counter].valid=1;
									cache[set_clear][counter].tag=tag_clear;
									cache[set_clear][counter].dirty=1;
									cache[set_clear][counter].lru=0;
									lruIncrement(assoc_number,cache[set_clear]);							
									print_action(memaddress, 1,processor_to_cache);	
									break;					
								}					
								counter++;					
							}
						}
						
					}
				}				
				
				
			}else if(op==4){//beq	
				
				stateptr.pc = stateptr.pc+1;
		
				if(offDest+stateptr.pc>haltLine){
					printf("ERROR: <Cannot jump to the memory address under 'halt' instruction>\n");
					exit(-1);
				}
				
				if(stateptr.reg[rb]==stateptr.reg[ra]){
					stateptr.pc = stateptr.pc+offDest;
				}
				
			}else if(op==5){//jalr

				int jTypeUnused = dataFromcache&0xFFFF;
				if(jTypeUnused!=0){
					printf("ERROR: <J-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;				
				if(ra==0){
					printf("ERROR: <$reg0 must be 0 forever>\n");
					exit(-1);
				}
				
				if(stateptr.reg[rb]>haltLine){
					printf("ERROR: <Cannot jump to the memory address under 'halt' instruction>\n");
					exit(-1);
				}
				
				stateptr.reg[ra] = stateptr.pc;
				stateptr.pc  = stateptr.reg[rb];
				
			}else if(op==6){//halt
				int oTypeUnused = dataFromcache&0x1FFFFF;
				if(oTypeUnused!=0){
					printf("ERROR: <O-type unused bits must be all zeros>\n");
					exit(-1);
				}
				for(int i=0;i<sets_number;i++){
					for(int j=0;j<assoc_number;j++){
						if(cache[i][j].dirty==1){
							int temp3 = (cache[i][j].tag<<(sai+eta))+(i<<(eta));
							for(int k=0;k<block_number;k++){
								stateptr.mem[temp3+k] = cache[i][j].block[k];
							}
							print_action(temp3,block_number,cache_to_memory);
						}
						cache[i][j].valid=0;
					}
				}
				break;
			}else if(op==7){//noop
				int oTypeUnused = dataFromcache&0x1FFFFF;
				if(oTypeUnused!=0){
					printf("ERROR: <O-type unused bits must be all zeros>\n");
					exit(-1);
				}
				stateptr.pc = stateptr.pc+1;
			}else{
				printf("ERROR: <Undefined opcode>\n");
				exit(-1);
			}

		}

	}
	return 0;
}

