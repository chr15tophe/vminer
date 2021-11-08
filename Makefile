CC=gcc
CFLAGS=-Wall -pedantic -Ofast -funroll-loops -march=native

all: bin/vminer

bin/vminer: src/main.c
	@mkdir -p bin
	gcc ${CFLAGS} -o bin/vminer src/main.c
