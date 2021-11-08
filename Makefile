CC=gcc
CFLAGS=-Wall -pedantic -Ofast -funroll-loops -march=native

all: bin/miner

bin/miner: src/main.c
	@mkdir -p bin
	gcc ${CFLAGS} -o bin/miner src/main.c

bin/miner.exe: src/main.c
	@mkdir -p bin
	x86_64-w64-mingw32-gcc ${CFLAGS} -o bin/miner.exe src/main.c
