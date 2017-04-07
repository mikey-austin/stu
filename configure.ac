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
AC_PATH_PROG([VALGRIND], [valgrind])
AC_PATH_PROG([DIFF], [diff])
AC_PATH_PROG([BASENAME], [basename])
AC_PATH_PROG([BASH], [bash])
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_PROG_LEX
AC_PROG_YACC
AC_TYPE_SIZE_T
AM_PROG_CC_C_O
AC_FUNC_STRNLEN

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
    src/libstu/libstu.pc
    check/Makefile
])

# Executables
AC_CONFIG_FILES([
    check/runner.sh
    check/test_prog_runner.sh
], [chmod +x check/*.sh])

AC_OUTPUT
