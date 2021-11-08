CC=gcc
CFLAGS=-Wall -pedantic -O3 -funroll-loops -march=native

all: bin/main

bin/main: src/main.c
	gcc ${CFLAGS} -o bin/main src/main.c
