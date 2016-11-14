CC=cc
CFLAGS=-g -Wall $(shell pkg-config --cflags libedit)
LDFLAGS=$(shell pkg-config --libs libedit)

all: stutter

stutter: lex.yy.o stutter.tab.o sv.o parse.o main.o hash.o env.o builtins.o
	$(CC) -o stutter lex.yy.o stutter.tab.o sv.o parse.o main.o hash.o env.o builtins.o $(LDFLAGS)

lex.yy.o: stutter.tab.h stutter.l sv.h
	flex --header-file=lex.yy.h stutter.l
	$(CC) $(CFLAGS) -c lex.yy.c

stutter.tab.h: stutter.tab.o

stutter.tab.o: stutter.y sv.h
	bison -d stutter.y
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

clean:
	rm -f lex.yy.[ch] stutter.tab.[ch] stutter *.o
