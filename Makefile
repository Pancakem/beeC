CC = clang
files = main.c parser.c type.c tokenizer.c codegen.c
program = compiler

build:
	$(CC) $(files) -o $(program) -O3 -Wall -pedantic

clean:
	rm $(program) *.gch
