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

#ifndef BUILTINS_DEFINED
#define BUILTINS_DEFINED

#include "gc.h"
#include "env.h"
#include "sv.h"

struct Stu;

extern void Builtin_init(struct Stu *);
extern Sv *Builtin_add(struct Stu *, Env *, Sv *);
extern Sv *Builtin_sub(struct Stu *, Env *, Sv *);
extern Sv *Builtin_mul(struct Stu *, Env *, Sv *);
extern Sv *Builtin_div(struct Stu *, Env *, Sv *);
extern Sv *Builtin_quote(struct Stu *, Env *, Sv *);
extern Sv *Builtin_def(struct Stu *, Env *, Sv *);
extern Sv *Builtin_cons(struct Stu *, Env *, Sv *);
extern Sv *Builtin_list(struct Stu *, Env *, Sv *);
extern Sv *Builtin_lambda(struct Stu *, Env *, Sv *);
extern Sv *Builtin_defmacro(struct Stu *, Env *, Sv *);
extern Sv *Builtin_macroexpand_1(struct Stu *, Env *, Sv *);
extern Sv *Builtin_macroexpand(struct Stu *, Env *, Sv *);
extern Sv *Builtin_progn(struct Stu *, Env *, Sv *);
extern Sv *Builtin_eval(struct Stu *, Env *, Sv *);
extern Sv *Builtin_car(struct Stu *, Env *, Sv *);
extern Sv *Builtin_cdr(struct Stu *, Env *, Sv *);
extern Sv *Builtin_reverse(struct Stu *, Env *, Sv *);
extern Sv *Builtin_if(struct Stu *, Env *, Sv*);
extern Sv *Builtin_eq(struct Stu *, Env *, Sv*);
extern Sv *Builtin_gt(struct Stu *, Env *, Sv*);
extern Sv *Builtin_lt(struct Stu *, Env *, Sv*);
extern Sv *Builtin_gte(struct Stu *, Env *, Sv*);
extern Sv *Builtin_lte(struct Stu *, Env *, Sv*);
extern Sv *Builtin_read(struct Stu *, Env *, Sv*);
extern Sv *Builtin_print(struct Stu *, Env *, Sv*);

#endif
