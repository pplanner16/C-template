# makefile for template
#
# Predefined Variables  
# CC compiler, default gcc
# CFLAGS compiler flags

CC=gcc
CFLAGS=-ansi -pedantic -Wall -Wextra -Og -g

template:

.PHONY: clean test

clean:
	$(RM) template *.o

test:
	clear
	cppcheck --enable=all --inconclusive --std=c89 template.c
	valgrind --leak-check=yes ./template test

