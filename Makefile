CC = gcc
src = main.c parser.c type.c tokenizer.c codegen.c
inc = parser.h type.h tokenizer.h codege.h
program = compiler

build:
	$(CC) -I inc $(src) -o $(program) -g -Wall

clean:
	rm $(program) *.gch
