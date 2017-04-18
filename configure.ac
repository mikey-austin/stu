AC_PREREQ([2.63])
AC_INIT([stu], [0.1.0], [mikey@jackiemclean.net])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE
LT_INIT([dlopen])
AC_CONFIG_SRCDIR([src/libstu/stu.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
PKG_INSTALLDIR

AC_PROG_CC
AC_PROG_INSTALL
AC_CHECK_FUNCS([strdup])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([unistd.h])
AC_PATH_PROG([POD2MAN], [pod2man])
AC_PATH_PROG([POD2HTML], [pod2html])
AC_PATH_PROG([VALGRIND], [valgrind])
AC_PATH_PROG([DIFF], [diff])
AC_PATH_PROG([BASENAME], [basename])
AC_PATH_PROG([BASH], [bash])
AC_PATH_PROG([PERL], [perl])
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_PROG_LEX
AC_PROG_YACC
AC_TYPE_SIZE_T
AM_PROG_CC_C_O
AC_FUNC_STRNLEN

# Documentation generation.
AC_ARG_VAR([DOC_CSS], [CSS styles for HTML documentation])
AS_IF([test -z ${DOC_CSS}], [DOC_CSS="man.css"])

# Memory allocation configuration.
AC_ARG_VAR([ALLOCATOR], [Memory allocator module])
AS_IF([test -z ${ALLOCATOR} || "x${ALLOCATOR}" = "xsystem" ], [
    AC_DEFINE([ALLOC_SYSTEM], 1, [use standard system allocator])
])
AS_IF([test "x${ALLOCATOR}" = "xslab" ], [
    AC_DEFINE([ALLOC_SLAB], 1, [use slab memory allocator])
])

# Only relevant if the slab allocator is specified above.
AC_ARG_VAR([SLAB_SIZE], [Default slab size for the slab allocator])
AS_IF([test -z ${SLAB_SIZE} ], [
    AC_DEFINE([SLAB_SIZE], 1000, [slab allocator slab size])
])

# Readline support for repl.
PKG_CHECK_MODULES([LIBEDIT], [libedit])
AC_CHECK_HEADERS([editline/readline.h])
CFLAGS="${CFLAGS} ${LIBEDIT_CFLAGS}"
LIBS="${LIBS} ${LIBEDIT_LIBS}"

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/repl/Makefile
    src/libstu/Makefile
    src/libstu/alloc/Makefile
    src/libstu/libstu.pc
    check/Makefile
    docs/Makefile
])

# Executables
AC_CONFIG_FILES([
    check/runner.sh
    check/test_prog_runner.sh
    docs/extract_pod.pl
], [chmod +x check/*.sh docs/*.pl])

AC_OUTPUT
