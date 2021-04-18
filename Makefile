CC = gcc
include = parser.h type.h tokenizer.h codegen.h
files = main.c parser.c type.c tokenizer.c codegen.c
program = compiler

build:
	$(CC) $(files) $(include) -o $(program) -O3

clean:
	rm $(program) *.gch
