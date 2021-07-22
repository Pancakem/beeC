CC = gcc
files = main.c parser.c type.c tokenizer.c codegen.c util.c
incs = parser.h type.h tokenizer.h codege.h util.h
program = compiler

build:
	$(CC) -I incs $(files) -o $(program) -g -Wall

clean:
	rm $(program) *.gch
