#! /bin/bash

CFLAGS=-I.
gcc -D_GNU_SOURCE -pedantic -std=c89 -c coroutine.c -o coroutine.o
gcc -D_GNU_SOURCE -pedantic -std=c89 -c test.c -o test.o
gcc -o test test.o coroutine.o
rm -rf *.o
