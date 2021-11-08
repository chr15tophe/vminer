CC=gcc
CFLAGS=-Wall -pedantic -Ofast -funroll-loops -march=native

all: bin/main

bin/main: src/main.c
	@mkdir -p bin
	gcc ${CFLAGS} -o bin/main src/main.c

bin/main.exe: src/main.c
	@mkdir -p bin
	x86_64-w64-mingw32-gcc ${CFLAGS} -o bin/main src/main.c
