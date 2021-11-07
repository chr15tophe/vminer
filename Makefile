.DEFAULT_GOAL := bin/vminer

bin/vminer: bin/main.o bin/sha256.o
	@mkdir -p bin
	@gcc -Wall -pedantic bin/main.o bin/sha256.o -o bin/vminer

bin/main.o: src/main.c
	@mkdir -p bin
	@gcc -c -Wall -pedantic src/main.c -o bin/main.o

bin/sha256.o: src/sha256.c
	@mkdir -p bin
	@gcc -c -Wall -pedantic src/sha256.c -o bin/sha256.o
