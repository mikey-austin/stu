#include_HEADERS = stu.h
noinst_HEADERS = builtins.h env.h gc.h hash.h parse.h sv.h svlist.h symtab.h utils.h

lib_LTLIBRARIES = libstu.la
libstu_la_SOURCES = parser.y lexer.l builtins.c env.c gc.c hash.c parse.c sv.c svlist.c symtab.c utils.c
libstu_la_LDFLAGS = -version-info 0:0:0

# Flex/Bison settings.
BUILT_SOURCES = parser.h
AM_YFLAGS = -d
AM_LFLAGS = --header-file=lex.yy.h
ACLOCAL_AMFLAGS = -I m4
