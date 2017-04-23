/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef STU_DEFINED
#define STU_DEFINED

#include <stdio.h>

/**
 * =head1 NAME
 *
 * B<libstu> provides the core routines for working with the L<stu> interpreter.
 *
 * =head1 SYNOPSIS
 *
 * Include the following header accordingly:
 *
 * =over 4
 *
 * B<#include <libstu/stu.h>>
 *
 * =back
 *
 * And the relevant build flags, which may be obtained via I<pkg-config>; e.g.
 *
 * =over 4
 *
 * B<$ pkg-config --cflags --libs libstu>
 *
 * =back
 *
 * =head1 DESCRIPTION
 *
 * The interpreter I<Stu> object encapsulates all interpreter state information;
 * more than one instance may be used in the same program.
 *
 * All returned I<StuVal> objects are automatically I<exempt> from garbage
 * collection. Once a I<StuVal> object is no longer required, it must be explicitly
 * released via I<Stu_release_val>.
 *
 * =head1 STRUCTURES
 *
 * The B<libstu> routines all operate on the following structures.
 *
 */

/**
 * =head2 StuVal
 *
 * The main structure encapsulating the atomic lisp object. The contents
 * of this structure are intentionally opaque.
 *
 */
typedef struct Sv StuVal;

/**
 * =head2 Stu
 *
 * The main L<stu> interpreter structure.
 *
 */
typedef struct Stu Stu;

/**
 * =head2 NIL
 *
 * The singleton NIL object. This object is initialized in the first call
 * to I<Stu_new>; any subsequent calls will make use of the same instance.
 *
 */

extern StuVal *Sv_nil;
#define NIL Sv_nil

/**
 * =head1 FUNCTIONS
 *
 * The following functions comprise the public B<libstu> interface.
 *
 */

/**
 * =head2 Stu *Stu_new(void)
 *
 * Create and initialize an interpreter instance.
 *
 */
extern Stu *Stu_new(void);

/**
 * =head2 void Stu_destroy(Stu **I<stu>)
 *
 * Destroy an interpreter instance created with I<Stu_new>.
 *
 */
extern void Stu_destroy(Stu **);

/**
 * =head2 StuVal *Stu_eval_file(Stu *I<stu>, const char *I<file>)
 *
 * Eval a NUL-terminated file path and return the result as a B<StuVal>.
 * The returned B<StuVal> object is managed by the garbage collector and need
 * not be explicitly cleaned.
 *
 */
extern StuVal *Stu_eval_file(Stu *, const char *);

/**
 * =head2 StuVal *Stu_eval_buf(Stu *I<stu>, const char *I<buf>)
 *
 * Same as I<Stu_eval_file>, but parse the NUL-terminated buffer instead.
 *
 */
extern StuVal *Stu_eval_buf(Stu *, const char *);

/**
 * =head2 int Stu_is_valid_form(Stu *I<stu>, const char *I<buf>)
 *
 * Returns 1 if the form contained in I<buf> is syntactically valid
 *         0 if the form is invalid
 *        -1 if the form is valid so far, but incomplete
 *
 */
extern int Stu_is_valid_form(Stu *, const char *);

/**
 * =head2 void Stu_release_val(Stu *I<stu>, StuVal *I<val>)
 *
 * Release the specified I<val> back to the mercy of the garbage collector,
 * to be cleaned up at some unspecified time in the future.
 *
 */
extern void Stu_release_val(Stu *, StuVal *);

/**
 * =head2 void Stu_dump_val(Stu *I<stu>, StuVal *I<val>, FILE *I<out>)
 *
 * Dump the string representation of I<val> to I<out> in the context of the
 * I<stu> interpreter instance.
 *
 */
extern void Stu_dump_val(Stu *, StuVal *, FILE *);

/**
 * =head2 void Stu_dump_stats(Stu *I<stu>, FILE *I<out>)
 *
 * Dump assorted interpreter stats to I<out>.
 *
 */
extern void Stu_dump_stats(Stu *, FILE *);

/**
 * =head1 AUTHOR
 *
 * B<stu> is written by Mikey Austin, Dmitry Petrov and others.
 *
 * =head1 SEE ALSO
 *
 * L<stu> - cli
 *
 * =head1 COPYRIGHT AND LICENSE
 *
 * See the I<COPYING> file in the distribution for the details.
 *
 * =cut
 */

#endif
