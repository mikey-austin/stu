/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
 * Copyright (c) 2017 Raphael Sousa Santos <contact@raphaelss.com>
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

#ifndef BUILTINS_DEFINED
#define BUILTINS_DEFINED

typedef struct Stu Stu;
typedef struct Env Env;
typedef struct Sv Sv;

extern void Builtin_init(Stu *);
extern Sv *Builtin_add(Stu *, Env *, Sv **);
extern Sv *Builtin_sub(Stu *, Env *, Sv **);
extern Sv *Builtin_mul(Stu *, Env *, Sv **);
extern Sv *Builtin_div(Stu *, Env *, Sv **);
extern Sv *Builtin_quote(Stu *, Env *, Sv **);
extern Sv *Builtin_def(Stu *, Env *, Sv **);
extern Sv *Builtin_cons(Stu *, Env *, Sv **);
extern Sv *Builtin_list(Stu *, Env *, Sv **);
extern Sv *Builtin_lambda(Stu *, Env *, Sv **);
extern Sv *Builtin_defmacro(Stu *, Env *, Sv **);
extern Sv *Builtin_macroexpand_1(Stu *, Env *, Sv **);
extern Sv *Builtin_macroexpand(Stu *, Env *, Sv **);
extern Sv *Builtin_progn(Stu *, Env *, Sv **);
extern Sv *Builtin_eval(Stu *, Env *, Sv **);
extern Sv *Builtin_car(Stu *, Env *, Sv **);
extern Sv *Builtin_cdr(Stu *, Env *, Sv **);
extern Sv *Builtin_reverse(Stu *, Env *, Sv **);
extern Sv *Builtin_if(Stu *, Env *, Sv **);
extern Sv *Builtin_eq(Stu *, Env *, Sv **);
extern Sv *Builtin_gt(Stu *, Env *, Sv **);
extern Sv *Builtin_lt(Stu *, Env *, Sv **);
extern Sv *Builtin_gte(Stu *, Env *, Sv **);
extern Sv *Builtin_lte(Stu *, Env *, Sv **);
extern Sv *Builtin_read(Stu *, Env *, Sv **);
extern Sv *Builtin_print(Stu *, Env *, Sv **);
extern Sv *Builtin_vector(Stu *, Env *, Sv **);
extern Sv *Builtin_tuple_constructor(Stu *, Env *, Sv **);
extern Sv *Builtin_size(Stu *, Env *, Sv **);
extern Sv *Builtin_at(Stu *, Env *, Sv **);
extern Sv *Builtin_type_of(Stu *, Env *, Sv **);
#endif
