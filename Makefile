all: stutter

stutter: lex.yy.o stutter.tab.o sv.o parse.o
	cc -o stutter lex.yy.o stutter.tab.o sv.o parse.o

lex.yy.o: stutter.tab.h stutter.l sv.h
	flex --header-file=lex.yy.h stutter.l
	cc -c lex.yy.c

stutter.tab.h: stutter.tab.o

stutter.tab.o: stutter.y sv.h
	bison -d stutter.y
	cc -c stutter.tab.c

sv.o: sv.c sv.h
	cc -c sv.c

parse.o: parse.c parse.h stutter.tab.h sv.h
	cc -c parse.c

clean:
	rm -f lex.yy.[ch] stutter.tab.[ch] stutter *.o
