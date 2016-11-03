all: stutter

stutter: lex.yy.o stutter.tab.o sv.o
	flex stutter.l
	bison -d stutter.y
	cc -o stutter lex.yy.o stutter.tab.o sv.o

lex.yy.o: stutter.l sv.h
	flex stutter.l
	cc -c lex.yy.c

stutter.tab.o: stutter.y sv.h
	bison -d stutter.y
	cc -c stutter.tab.c

sv.o: sv.c sv.h
	cc -c sv.c

clean:
	rm -f lex.yy.c stutter.tab.c stutter.tab.h stutter *.o
