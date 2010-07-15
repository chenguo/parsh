all: parsh

parsh: main.o parse.o
	gcc -lpthread -o ../parsh main.o parse.o

main.o: main.c
	gcc -O -c main.c

parser.o: parser.c parse.h
	gcc -O -c parse.c
