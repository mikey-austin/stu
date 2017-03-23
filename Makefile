CC=cc
BISON=bison
CFLAGS=-g3 -Wall $(shell pkg-config --cflags libedit) -O0 -std=gnu99
LDFLAGS=$(shell pkg-config --libs libedit)
BASH=/bin/bash
WITH_VALGRIND=-m

all: stu

# run make macosx_deps to get dependencies necessary for mac os x build
with_brew: BISON=$(shell brew --prefix bison)/bin/bison
with_brew: stu

stu: lex.yy.o stu.tab.o sv.o parse.o main.o hash.o env.o builtins.o utils.o gc.o symtab.o svlist.o
	$(CC) -o stu lex.yy.o stu.tab.o sv.o parse.o main.o hash.o env.o builtins.o utils.o gc.o symtab.o svlist.o $(LDFLAGS)

test: stu
	cd test; $(BASH) ./runner.sh $(WITH_VALGRIND) -f ../stu -l ../stdlib.stu

repl: stu
	valgrind -q --error-exitcode=1 --leak-check=full --show-leak-kinds=definite,possible --tool=memcheck ./stu -l stdlib.stu -dr

lex.yy.o: stu.tab.h stu.l sv.h
	flex --header-file=lex.yy.h stu.l
	$(CC) $(CFLAGS) -c lex.yy.c

stu.tab.h: stu.tab.o

stu.tab.o: stu.y sv.h
	$(BISON) -d stu.y
	$(CC) $(CFLAGS) -c stu.tab.c

sv.o: sv.c sv.h env.h
	$(CC) $(CFLAGS) -c sv.c

svlist.o: svlist.c svlist.h
	$(CC) $(CFLAGS) -c svlist.c

main.o: main.c parse.h sv.h
	$(CC) $(CFLAGS) -c main.c

parse.o: parse.c parse.h stu.tab.h sv.h
	$(CC) $(CFLAGS) -c parse.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

env.o: env.c env.h hash.h
	$(CC) $(CFLAGS) -c env.c

builtins.o: builtins.c env.h sv.h
	$(CC) $(CFLAGS) -c builtins.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

gc.o: gc.c gc.h
	$(CC) $(CFLAGS) -c gc.c

symtab.o: symtab.c symtab.h hash.h
	$(CC) $(CFLAGS) -c symtab.c

macosx_deps:
	brew install bison

clean:
	rm -f lex.yy.[ch] stu.tab.[ch] stu *.o

.PHONY: test repl
