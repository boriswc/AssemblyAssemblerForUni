all:
	gcc		-Wall -ansi assembler.c label.c parser.c file_management.c -pedantic -o assembler -g
