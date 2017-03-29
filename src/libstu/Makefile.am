ACLOCAL_AMFLAGS = -I m4
include_HEADERS = stu.h sv.h svlist.h
noinst_HEADERS = builtins.h env.h gc.h hash.h symtab.h utils.h

lib_LTLIBRARIES = libstu.la
libstu_la_SOURCES = parser.y lexer.l builtins.c env.c gc.c hash.c stu.c sv.c svlist.c symtab.c utils.c
libstu_la_LDFLAGS = -version-info 0:0:0

# Flex/Bison settings.
BUILT_SOURCES = parser.h lexer.h
AM_YFLAGS = -d

CLEANFILES = lexer.h
lexer.h: lexer.l
	$(LEX) --header-file=$@ -o /dev/null $<
