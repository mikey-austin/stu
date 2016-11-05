CC=cc
CFLAGS=-g -Wall $(shell pkg-config --cflags libedit)
LDFLAGS=$(shell pkg-config --libs libedit)

all: stutter

stutter: lex.yy.o stutter.tab.o sv.o parse.o main.o
	$(CC) -o stutter lex.yy.o stutter.tab.o sv.o parse.o main.o $(LDFLAGS)

lex.yy.o: stutter.tab.h stutter.l sv.h
	flex --header-file=lex.yy.h stutter.l
	$(CC) $(CFLAGS) -c lex.yy.c

stutter.tab.h: stutter.tab.o

stutter.tab.o: stutter.y sv.h
	bison -d stutter.y
	$(CC) $(CFLAGS) -c stutter.tab.c

sv.o: sv.c sv.h
	$(CC) $(CFLAGS) -c sv.c

main.o: main.c parse.h sv.h
	$(CC) $(CFLAGS) -c main.c

parse.o: parse.c parse.h stutter.tab.h sv.h
	$(CC) $(CFLAGS) -c parse.c

clean:
	rm -f lex.yy.[ch] stutter.tab.[ch] stutter *.o
