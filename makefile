  # build an executable named myprog from myprog.c
  all: sema.c
	  gcc -w sema.c -Wall -pedantic -std=c99 -D_XOPEN_SOURCE=700 -pthread
