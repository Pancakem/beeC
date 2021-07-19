CC = clang
files = main.c parser.c type.c tokenizer.c codegen.c
incs = parser.h type.h tokenizer.h codege.h 
program = compiler

build:
	$(CC) -I incs $(files) -o $(program) -O3 -Wall -pedantic -v

clean:
	rm $(program) *.gch
