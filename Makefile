CC=cc
BISON=bison
CFLAGS=-g -Wall $(shell pkg-config --cflags libedit) -O0
LDFLAGS=$(shell pkg-config --libs libedit)
BASH=/bin/bash

all: stutter

# run make macosx_deps to get dependencies necessary for mac os x build
with_brew: BISON=$(shell brew --prefix bison)/bin/bison
with_brew: stutter

stutter: lex.yy.o stutter.tab.o sv.o parse.o main.o hash.o env.o builtins.o utils.o
	$(CC) -o stutter lex.yy.o stutter.tab.o sv.o parse.o main.o hash.o env.o builtins.o utils.o $(LDFLAGS)

test: stutter
	cd test; $(BASH) ./runner.sh ../stutter

lex.yy.o: stutter.tab.h stutter.l sv.h
	flex --header-file=lex.yy.h stutter.l
	$(CC) $(CFLAGS) -c lex.yy.c

stutter.tab.h: stutter.tab.o

stutter.tab.o: stutter.y sv.h
	$(BISON) -d stutter.y
	$(CC) $(CFLAGS) -c stutter.tab.c

sv.o: sv.c sv.h env.h
	$(CC) $(CFLAGS) -c sv.c

main.o: main.c parse.h sv.h
	$(CC) $(CFLAGS) -c main.c

parse.o: parse.c parse.h stutter.tab.h sv.h
	$(CC) $(CFLAGS) -c parse.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

env.o: env.c env.h hash.h
	$(CC) $(CFLAGS) -c env.c

builtins.o: builtins.c env.h sv.h
	$(CC) $(CFLAGS) -c builtins.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

macosx_deps:
	brew install bison

clean:
	rm -f lex.yy.[ch] stutter.tab.[ch] stutter *.o

.PHONY: test
