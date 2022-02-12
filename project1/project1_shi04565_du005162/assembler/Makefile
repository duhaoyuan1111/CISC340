CC=gcc

all: assembler.o
	$(CC) assembler.o -o assembler.asm
assembler: assembler.c
	$(CC) -c assembler.c
clean:
	rm *.o assembler.asm