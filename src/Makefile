all: parsh

parsh: main.o parser.o
	gcc -o parsh main.o parser.o

main.o: main.c
	gcc -O -c main.c

parser.o: parser.c parser.h
	gcc -O -c parser.c