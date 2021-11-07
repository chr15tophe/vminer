.DEFAULT_GOAL := bin/vminer

CFLAGS=-Wall -pedantic -O3 -funroll-loops -march=native

bin/vminer: bin/main.o bin/sha256.o
	@mkdir -p bin
	@gcc $(CFLAGS) bin/main.o bin/sha256.o -o bin/vminer

bin/vminer.exe: bin/main.wo bin/sha256.wo
	@mkdir -p bin
	@x86_64-w64-mingw32-gcc $(CFLAGS) bin/main.wo bin/sha256.wo -o bin/vminer.exe

bin/main.o: src/main.c
	@mkdir -p bin
	@gcc -c $(CFLAGS) src/main.c -o bin/main.o

bin/main.wo: src/main.c
	@mkdir -p bin
	@x86_64-w64-mingw32-gcc -c $(CFLAGS) src/main.c -o bin/main.wo

bin/sha256.o: src/sha256.c
	@mkdir -p bin
	@gcc -c $(CFLAGS) src/sha256.c -o bin/sha256.o

bin/sha256.wo: src/sha256.c
	@mkdir -p bin
	@x86_64-w64-mingw32-gcc -c $(CFLAGS) src/sha256.c -o bin/sha256.wo
