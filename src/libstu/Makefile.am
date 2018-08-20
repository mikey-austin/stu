SUBDIRS = alloc
ACLOCAL_AMFLAGS = -I m4
include_HEADERS = stu.h
noinst_HEADERS = builtins.h env.h gc.h hash.h native_func.h symtab.h utils.h special_form.h sv.h stu_private.h types.h try.h call_stack.h mod.h

lib_LTLIBRARIES = libstu.la
libstu_la_LIBADD = alloc/liballoc.la
libstu_la_SOURCES = parser.y lexer.l builtins.c env.c gc.c hash.c native_func.c special_form.c stu.c sv.c symtab.c types.c utils.c try.c call_stack.c mod.c
libstu_la_LDFLAGS = -version-info 0:0:0

pkgconfig_DATA = libstu.pc

# Flex/Bison settings.
BUILT_SOURCES = parser.h lexer.h
AM_YFLAGS = -d

CLEANFILES = lexer.h
lexer.h: lexer.l
	$(LEX) --header-file=$@ -o /dev/null $<
