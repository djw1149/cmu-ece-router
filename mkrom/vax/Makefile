CFLAGS = -g
CC = cc
MACH = vax

all: mkrom

mkrom: mkrom.o getopt.o
	cc -o mkrom mkrom.o getopt.o

mkrom.o: ../mkrom.c ../mkrom.h ../keyw.h
	cc -c -g ../mkrom.c

getopt.o: ../getopt.c
	cc -c -g ../getopt.c 
