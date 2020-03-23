CC=gcc-9
CFLAGS=-Wall
MP=-fopenmp
DEP=ppmFile.h pmmFile.c

all: hw4
	
%.o: %.c $(DEP)
	$(CC) -g -o $@ $< $(CFLAGS)
hw4: hw4.c
	$(CC) $(MP) -g -o $@ $^ $(CFLAGS)
clean:
	rm hw4
