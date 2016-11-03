all: stutter

stutter: stutter.l stutter.y
	flex stutter.l
	bison -d stutter.y
	cc -o stutter lex.yy.c stutter.tab.c

clean:
	rm -f lex.yy.c stutter.tab.c stutter.tab.h stutter
