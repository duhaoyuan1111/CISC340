#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char **argv)
{
  FILE* as;
  FILE* mc;
  char read[160];
  int c;
  int i,j;
  int pack[3];
  char *ivalue = NULL;
  char *ovalue = NULL;
  int next;
  unsigned long long int packed;
  int printfile;
 
  while ((c = getopt (argc, argv, "i:o:")) != -1)
    switch (c)
      {
      case 'i': 
	    ivalue=optarg;
        break;
	  case 'o':
		ovalue=optarg;
		break;
      default:
        abort ();
      }

  char label[51][7];
  char allLine[51][40];
  
  for(j=0;j<50;j++){
	  memset(label[j],'\0',sizeof(label[j]));
  }
  
  if(ivalue !=NULL){
    as=fopen(ivalue,"r");
	j=0;
	while(fgets(read,sizeof(read),as)!= NULL){
		i=0;
		if(read[0]!=' ' && read[0]!='\t'){ // have label
			
			while(read[i]!=' ' && read[i]!='\t'){
				if(i==0 && read[i]!='a' && read[i]!='b' && read[i]!='c' && read[i]!='d' && read[i]!='e' && read[i]!='f'
					&& read[i]!='g' && read[i]!='h' && read[i]!='i' && read[i]!='j' && read[i]!='k' && read[i]!='l'
					&& read[i]!='m' && read[i]!='n' && read[i]!='o' && read[i]!='p' && read[i]!='q' && read[i]!='r'
					&& read[i]!='s' && read[i]!='t' && read[i]!='u' && read[i]!='v' && read[i]!='w' && read[i]!='x'
					&& read[i]!='y' && read[i]!='z' && read[i]!='A' && read[i]!='B' && read[i]!='C' && read[i]!='D'
					&& read[i]!='E' && read[i]!='F' && read[i]!='G' && read[i]!='H' && read[i]!='I' && read[i]!='J'
					&& read[i]!='K' && read[i]!='L' && read[i]!='M' && read[i]!='N' && read[i]!='O' && read[i]!='P'
					&& read[i]!='Q' && read[i]!='R' && read[i]!='S' && read[i]!='T' && read[i]!='U' && read[i]!='V'
					&& read[i]!='W' && read[i]!='X' && read[i]!='Y' && read[i]!='Z'){
					printf("ERROR: <Labels start with a number or a weird character>\n");
					exit(-1);
				}else if(i!=0 && read[i]!='a' && read[i]!='b' && read[i]!='c' && read[i]!='d' && read[i]!='e' && read[i]!='f'
					&& read[i]!='g' && read[i]!='h' && read[i]!='i' && read[i]!='j' && read[i]!='k' && read[i]!='l'
					&& read[i]!='m' && read[i]!='n' && read[i]!='o' && read[i]!='p' && read[i]!='q' && read[i]!='r'
					&& read[i]!='s' && read[i]!='t' && read[i]!='u' && read[i]!='v' && read[i]!='w' && read[i]!='x'
					&& read[i]!='y' && read[i]!='z' && read[i]!='A' && read[i]!='B' && read[i]!='C' && read[i]!='D'
					&& read[i]!='E' && read[i]!='F' && read[i]!='G' && read[i]!='H' && read[i]!='I' && read[i]!='J'
					&& read[i]!='K' && read[i]!='L' && read[i]!='M' && read[i]!='N' && read[i]!='O' && read[i]!='P'
					&& read[i]!='Q' && read[i]!='R' && read[i]!='S' && read[i]!='T' && read[i]!='U' && read[i]!='V'
					&& read[i]!='W' && read[i]!='X' && read[i]!='Y' && read[i]!='Z' && read[i]!='0' && read[i]!='1'
					&& read[i]!='2' && read[i]!='3' && read[i]!='4' && read[i]!='5' && read[i]!='6' && read[i]!='7'
					&& read[i]!='8' && read[i]!='9'){
					printf("ERROR: <Labels have weird characters>\n");
					exit(-1);
				}else if(i>=6){
					printf("ERROR: <Labels are too long>\n");
					exit(-1);
				}else{
					label[j][i]=read[i];
				}
				i++;
			}
		}
		j=j+1;
	}
	
	int tt;
	int oo;
	for(tt=0;tt<49;tt++){ //check duplicate labels
		for(oo=tt+1;oo<50;oo++){
			if(strcmp(label[tt],label[oo])==0 && label[tt][0]!='\0'){
				printf("ERROR: <Cannot have duplicate labels>\n");
				exit(-1);
			}
		}
	}
	
	for(tt=0;tt<49;tt++){
		if(strcmp(label[tt],"lw")==0 || strcmp(label[tt],"sw")==0 ||
		strcmp(label[tt],"add")==0 || strcmp(label[tt],"nand")==0 ||
		strcmp(label[tt],"noop")==0 || strcmp(label[tt],"halt")==0 ||
		strcmp(label[tt],"jalr")==0 || strcmp(label[tt],"beq")==0){
			printf("ERROR: <Cannot use instructions as your label's name>\n");
			exit(-1);
			
		}
	}
	
	fclose(as);
	as=fopen(ivalue,"r");
	char read1[160];
	int counter=0;
	int hh;
	for(hh=0;hh<50;hh++){
	  memset(allLine[hh],'\0',sizeof(allLine[hh]));
	}
	while(fgets(read1,sizeof(read1),as)!= NULL){
		
		i=0;
		while(read1[i]!=' '&&read1[i]!='\t'){ //where to start
			i++;
		}
		while(read1[i]==' '||read1[i]=='\t'){
			i++;
		}
		j=0;
		int check=1;
		while(check==1){
			if(read1[i]=='#' || read1[i]=='\n' || read1[i]=='\r'){
				check=0;
			}else{
				if(read1[i]!=' '&&read1[i]!='\t'){
					allLine[counter][j]=read1[i];
					j++;
				}
				i++;
			}
		}
		counter++;
	}
	
	int a;
	int b;
	long outcode[counter];
	int op;
	int rA;
	int rB;
	for(i=0;i<counter;i++){
		char temp[5][7];
		for(a=0;a<5;a++){
			memset(temp[a],'\0',sizeof(temp[a]));
		}

		if(allLine[i][0]=='l' || allLine[i][0]=='s'){//lw sw
			temp[0][0]=allLine[i][0];
			temp[0][1]=allLine[i][1];
			temp[1][0]=allLine[i][2];
			temp[2][0]=allLine[i][3];
			for(j=4;j<strlen(allLine[i]);j++){
				temp[3][j-4]=allLine[i][j];
			}
			
			if(strcmp(temp[0],"lw")!=0 && strcmp(temp[0],"sw")!=0){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			if(temp[1][0]!='0' && temp[1][0]!='1' && temp[1][0]!='2' && temp[1][0]!='3'
				&& temp[1][0]!='4' && temp[1][0]!='5' && temp[1][0]!='6' && temp[1][0]!='7' && temp[1][0]!='8' && temp[1][0]!='9'){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			int lwoffset;
			int y=0;
			int ty;
			for(b=0;b<50;b++){
				if(strcmp(label[b],temp[3])==0){
					lwoffset=b;
					y=y+1;
				}
			}
			if(temp[3][0]!='-' && temp[3][0]!='0' && temp[3][0]!='1' && temp[3][0]!='2' && temp[3][0]!='3'
				&& temp[3][0]!='4' && temp[3][0]!='5' && temp[3][0]!='6' && temp[3][0]!='7' && temp[3][0]!='8' && temp[3][0]!='9' && y==0){
				printf("ERROR: <Undefined labels in lw or sw>\n");
				exit(-1);
			}
			if(temp[3][0]=='-'){
				if(temp[3][1]!='1' && temp[3][1]!='2' && temp[3][1]!='3'
					&& temp[3][1]!='4' && temp[3][1]!='5' && temp[3][1]!='6' && temp[3][1]!='7' && temp[3][1]!='8' && temp[3][1]!='9'){
					printf("ERROR: <Undefined labels in lw or sw>\n");
					exit(-1);
				}
			}
			
			if(y==0){
				lwoffset=atoi(temp[3]);
			}
			if(lwoffset>32767 || lwoffset<-32768){
				printf("ERROR: <Offset_fields that don't fit in 16 bits in lw/sw>\n");
				exit(-1);
			}
			
			if(strcmp(temp[0],"lw")==0){
				op=2;
			}else{
				op=3;
			}
			
			rA=atoi(temp[1]);
			rB=atoi(temp[2]);

			long temp1=((u_int32_t)op<<22)|((u_int32_t)rA<<19)|((u_int32_t)rB<<16)|((int16_t)lwoffset);
			outcode[i]=temp1;
		}else if(allLine[i][0]=='b'){//beq
			temp[0][0]=allLine[i][0];
			temp[0][1]=allLine[i][1];
			temp[0][2]=allLine[i][2];
			temp[1][0]=allLine[i][3];
			temp[2][0]=allLine[i][4];
			for(j=5;j<strlen(allLine[i]);j++){
				temp[3][j-5]=allLine[i][j];
			}
			if(strcmp(temp[0],"beq")!=0){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			if(temp[1][0]!='0' && temp[1][0]!='1' && temp[1][0]!='2' && temp[1][0]!='3'
				&& temp[1][0]!='4' && temp[1][0]!='5' && temp[1][0]!='6' && temp[1][0]!='7' && temp[1][0]!='8' && temp[1][0]!='9'){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			
			int beqoffset;
			int yy=0;
			for(b=0;b<50;b++){
				if(strcmp(label[b],temp[3])==0){
					beqoffset=b-1-i;
					yy++;
				}
			}
			if(temp[3][0]!='-' && temp[3][0]!='0' && temp[3][0]!='1' && temp[3][0]!='2' && temp[3][0]!='3'
				&& temp[3][0]!='4' && temp[3][0]!='5' && temp[3][0]!='6' && temp[3][0]!='7' && temp[3][0]!='8' && temp[3][0]!='9' && yy==0){
				printf("ERROR: <Undefined labels in beq>\n");
				exit(-1);
			}
			if(temp[3][0]=='-'){
				if(temp[3][1]!='1' && temp[3][1]!='2' && temp[3][1]!='3'
					&& temp[3][1]!='4' && temp[3][1]!='5' && temp[3][1]!='6' && temp[3][1]!='7' && temp[3][1]!='8' && temp[3][1]!='9'){
					printf("ERROR: <Undefined labels in beq>\n");
					exit(-1);
				}
			}
			
			if(yy==0){
				beqoffset=atoi(temp[3]);
			}
			if(beqoffset>32767 || beqoffset<-32768){
				printf("ERROR: <Offset_fields that don't fit in 16 bits in beq>\n");
				exit(-1);
			}
			
			op=4;
			rA=atoi(temp[1]);
			rB=atoi(temp[2]);
			long temp2=((u_int32_t)op<<22)|((u_int32_t)rA<<19)|((u_int32_t)rB<<16)|((u_int16_t)beqoffset);

			outcode[i]=temp2;
		}else if(allLine[i][0]=='a'){//add
			temp[0][0]=allLine[i][0];
			temp[0][1]=allLine[i][1];
			temp[0][2]=allLine[i][2];
			temp[1][0]=allLine[i][3];
			temp[2][0]=allLine[i][4];
			for(j=5;j<strlen(allLine[i]);j++){
				temp[3][j-5]=allLine[i][j];
			}
			if(strcmp(temp[0],"add")!=0){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			if(temp[1][0]!='0' && temp[1][0]!='1' && temp[1][0]!='2' && temp[1][0]!='3'
				&& temp[1][0]!='4' && temp[1][0]!='5' && temp[1][0]!='6' && temp[1][0]!='7' && temp[1][0]!='8' && temp[1][0]!='9'){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			op=0;
			rA=atoi(temp[2]);
			rB=atoi(temp[3]);
			int destreg = atoi(temp[1]);
			long temp3=((u_int32_t)op<<22)|((u_int32_t)rA<<19)|((u_int32_t)rB<<16)|((u_int32_t)destreg);
			outcode[i]=temp3;
			
		}else if(allLine[i][0]=='j'){//jalr
			temp[0][0]=allLine[i][0];
			temp[0][1]=allLine[i][1];
			temp[0][2]=allLine[i][2];
			temp[0][3]=allLine[i][3];
			temp[1][0]=allLine[i][4];
			temp[2][0]=allLine[i][5];
			if(strcmp(temp[0],"jalr")!=0){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			if(temp[1][0]!='0' && temp[1][0]!='1' && temp[1][0]!='2' && temp[1][0]!='3'
				&& temp[1][0]!='4' && temp[1][0]!='5' && temp[1][0]!='6' && temp[1][0]!='7' && temp[1][0]!='8' && temp[1][0]!='9'){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			op=5;
			rA=atoi(temp[1]);
			rB=atoi(temp[2]);

			long temp4=((u_int32_t)op<<22)|((u_int32_t)rA<<19)|((u_int32_t)rB<<16);
			outcode[i]=temp4;
		}else if(allLine[i][0]=='h'){//halt
			temp[0][0]=allLine[i][0];
			temp[0][1]=allLine[i][1];
			temp[0][2]=allLine[i][2];
			temp[0][3]=allLine[i][3];
			if(strcmp(temp[0],"halt")!=0){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			if(allLine[i][4]!='\0'){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			op=6;
			long temp5=((u_int32_t)op<<22);
			outcode[i]=temp5;
		}else if(allLine[i][0]=='.'){//.fill
			temp[0][0]=allLine[i][0];
			temp[0][1]=allLine[i][1];
			temp[0][2]=allLine[i][2];
			temp[0][3]=allLine[i][3];
			temp[0][4]=allLine[i][4];
			for(j=5;j<strlen(allLine[i]);j++){
				temp[1][j-5]=allLine[i][j];
			}
			if(strcmp(temp[0],".fill")!=0){
				printf("ERROR: <Unrecognized opcodes>\n");
				exit(-1);
			}
			
			int filloffset;
			int yyy=0;
			for(b=0;b<50;b++){
				if(strcmp(label[b],temp[1])==0){
					filloffset=b;
					yyy++;
				}
			}
			if(temp[1][0]!='-' && temp[1][0]!='0' && temp[1][0]!='1' && temp[1][0]!='2' && temp[1][0]!='3'
				&& temp[1][0]!='4' && temp[1][0]!='5' && temp[1][0]!='6' && temp[1][0]!='7' && temp[1][0]!='8' && temp[1][0]!='9' && yyy==0){
				printf("ERROR: <Undefined labels in .fill>\n");
				exit(-1);
			}
			if(temp[1][0]=='-'){
				if(temp[1][1]!='1' && temp[1][1]!='2' && temp[1][1]!='3'
					&& temp[1][1]!='4' && temp[1][1]!='5' && temp[1][1]!='6' && temp[1][1]!='7' && temp[1][1]!='8' && temp[1][1]!='9'){
					printf("ERROR: <Undefined labels in .fill>\n");
					exit(-1);
				}
			}
			
			if(yyy==0){
				filloffset=atoi(temp[1]);
			}
			if(filloffset>32767 || filloffset<-32768){
				printf("ERROR: <Offset_fields that don't fit in 16 bits in .fill>\n");
				exit(-1);
			}
			long temp6=((int16_t)filloffset);
			outcode[i]=temp6;
		}else if(allLine[i][0]=='n'){
			if(allLine[i][1]=='a'){//nand
				temp[0][0]=allLine[i][0];
				temp[0][1]=allLine[i][1];
				temp[0][2]=allLine[i][2];
				temp[0][3]=allLine[i][3];
				temp[1][0]=allLine[i][4];
				temp[2][0]=allLine[i][5];
				for(j=6;j<strlen(allLine[i]);j++){
					temp[3][j-6]=allLine[i][j];
				}
				if(strcmp(temp[0],"nand")!=0){
					printf("ERROR: <Unrecognized opcodes>\n");
					exit(-1);
				}
				if(temp[1][0]!='0' && temp[1][0]!='1' && temp[1][0]!='2' && temp[1][0]!='3'
					&& temp[1][0]!='4' && temp[1][0]!='5' && temp[1][0]!='6' && temp[1][0]!='7' && temp[1][0]!='8' && temp[1][0]!='9'){
					printf("ERROR: <Unrecognized opcodes>\n");
					exit(-1);
				}
				op=1;
				rA=atoi(temp[2]);
				rB=atoi(temp[3]);
				int destreg1 = atoi(temp[1]);
				long temp7=((u_int32_t)op<<22)|((u_int32_t)rA<<19)|((u_int32_t)rB<<16)|((u_int32_t)destreg1);
				outcode[i]=temp7;
			}else{//noop
				temp[0][0]=allLine[i][0];
				temp[0][1]=allLine[i][1];
				temp[0][2]=allLine[i][2];
				temp[0][3]=allLine[i][3];
				if(strcmp(temp[0],"noop")!=0){
					printf("ERROR: <Unrecognized opcodes>\n");
					exit(-1);
				}
				if(allLine[i][4]!='\0'){
					printf("ERROR: <Unrecognized opcodes>\n");
					exit(-1);
				}
				op=7;
				long temp8=((u_int32_t)op<<22);	
				outcode[i]=temp8;
			}
		}else{
			printf("ERROR: <Unrecognized opcodes>\n");
			exit(-1);
		}
	}
	int k;
	if(ovalue !=NULL) {
		mc = fopen(ovalue,"w");
	    for(k=0;k<counter;k++){
			fprintf(mc,"%ld\n",outcode[k]);
		}
    }else{
		for(i=0;i<counter;i++){
			printf("%ld",outcode[i]);
			printf("\n");
		}
	}
  }
  return 0;
}