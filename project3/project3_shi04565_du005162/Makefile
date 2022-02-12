CC=gcc

all: simulator.o
	$(CC) simulator.o -o simulator.sim
simulator: simulator.c
	$(CC) -c simulator.c
clean:
	rm *.o simulator.sim