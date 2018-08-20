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

#ifndef SV_DEFINED
#define SV_DEFINED

#include <stdio.h>
#include <regex.h>

#include "gc.h"
#include "types.h"

#define SV_CONS_REGISTERS 2
#define SV_CAR_REG 0
#define SV_CDR_REG 1

#define IS_NIL(sv)   ((sv) && (sv)->type == SV_NIL ? 1 : ((sv) ? 0 : 1))
#define IS_MACRO(sv) ((sv) && (((sv)->type == SV_LAMBDA && ((sv)->val.ufunc->is_macro)) \
                               || ((sv)->type == SV_NATIVE_FUNC && Sv_native_func_is_macro(sv->val.func))))
#define CAR(sv)      ((sv) && (sv)->type == SV_CONS ? (sv)->val.reg[SV_CAR_REG] : NULL)
#define CDR(sv)      ((sv) && (sv)->type == SV_CONS ? (sv)->val.reg[SV_CDR_REG] : NULL)
#define CADR(sv)     ((sv) ? CAR(CDR((sv))) : NULL)
#define CADDR(sv)    ((sv) ? CAR(CDR(CDR((sv)))) : NULL)

/* Types of stu values. */
enum Sv_type {
    SV_NIL,
    SV_ERR,
    SV_SYM,
    SV_INT,
    SV_FLOAT,
    SV_RATIONAL,
    SV_BOOL,
    SV_STR,
    SV_CONS,
    SV_NATIVE_FUNC,
    SV_NATIVE_CLOS,
    SV_LAMBDA,
    SV_SPECIAL,
    SV_TUPLE,
    SV_TUPLE_CONSTRUCTOR,
    SV_FOREIGN,
    SV_REGEX
};

enum Sv_special_type {
    SV_SPECIAL_COMMA,
    SV_SPECIAL_COMMA_SPREAD,
    SV_SPECIAL_BACKQUOTE
};

/* Forward declarations. */
struct Gc;
struct Env;
struct Sv;
struct Stu;

typedef struct Sv *(*Sv_native_func_t)(struct Stu *, struct Env *, struct Sv **);

typedef struct Sv_tuple {
    Type type;
    struct Sv *values[];
} Sv_tuple;

typedef struct Sv_ufunc {
    struct Env *env;
    struct Sv *formals;
    struct Sv *body;
    short  is_macro;
} Sv_ufunc;

typedef struct Sv_special {
    enum Sv_special_type type;
    struct Sv *body;
} Sv_special;

typedef struct Sv_rational {
    long n;
    long d;
} Sv_rational;

typedef void (*Sv_foreign_destructor_t)(void *);

typedef struct Sv_foreign {
    void *obj;
    Sv_foreign_destructor_t destructor;
} Sv_foreign;

typedef struct Sv_re {
    char *spec;
    int icase;
    regex_t compiled;
} Sv_re;

/* Actual value container. */
union Sv_val {
    long i;
    double f;
    char *buf;
    Sv_rational rational;
    struct Sv_native_func *func;
    struct Sv_native_closure *clos;
    struct Sv_special *special;
    struct Sv *reg[SV_CONS_REGISTERS];
    struct Sv_ufunc *ufunc;
    struct Sv_tuple *tuple;
    struct Type tuple_constructor;
    struct Sv_foreign foreign;
    struct Sv_re re;
};

/* Core stu value. */
typedef struct Sv {
    struct Gc gc;
    enum Sv_type type;
    union Sv_val val;
} Sv;

extern Sv *Sv_new(struct Stu *, enum Sv_type);
extern Sv *Sv_new_int(struct Stu *, long);
extern Sv *Sv_new_float(struct Stu *, double);
extern Sv *Sv_new_rational(struct Stu *, long, long);
extern Sv *Sv_new_bool(struct Stu *, short);
extern Sv *Sv_new_str(struct Stu *, const char *);
extern Sv *Sv_new_sym(struct Stu *, const char *);
extern Sv *Sv_new_err(struct Stu *, const char *);
extern Sv *Sv_new_native_func(struct Stu *, Sv_native_func_t, unsigned, unsigned);
extern Sv *Sv_new_lambda(struct Stu *, struct Env *, Sv *, Sv *);
extern Sv *Sv_new_special(struct Stu *, enum Sv_special_type type, Sv *body);
extern Sv *Sv_new_vector(struct Stu *, Sv *);
extern Sv *Sv_new_tuple(struct Stu *, Type, Sv *);
extern Sv *Sv_new_tuple_constructor(struct Stu *, Type);
extern Sv *Sv_new_foreign(struct Stu *, void *, Sv_foreign_destructor_t);
extern Sv *Sv_new_regex(struct Stu *, const char *, int);

extern void Sv_dump(struct Stu *, Sv *sv, FILE *);
extern void Sv_destroy(struct Stu *, Sv **);
extern Sv *Sv_cons(struct Stu *, Sv *, Sv *);
extern Sv *Sv_reverse(struct Stu *, Sv *);
extern Sv *Sv_list(struct Stu *, Sv *);
extern Sv *Sv_copy(struct Stu *, Sv *);
extern Sv *Sv_expand(struct Stu *, Sv *);
extern Sv *Sv_expand_1(struct Stu *, Sv *);
extern void *Sv_get_foreign_obj(struct Stu *, Sv *);

extern Sv *Sv_eval(struct Stu *, struct Env *, Sv *);
extern Sv *Sv_eval_list(struct Stu *, struct Env *, Sv *);
extern Sv *Sv_eval_list_main_env(struct Stu *, struct Env *, Sv *);
extern Sv *Sv_eval_list_generic(struct Stu *, struct Env *, Sv *, Sv *(*eval)(struct Stu *, struct Env *, Sv *, struct Env **));
extern Sv *Sv_eval_special(struct Stu *, struct Env *, Sv *);
extern Sv *Sv_eval_special_cons(struct Stu *, struct Env *, Sv *);
extern Sv *Sv_eval_sexp(struct Stu *, struct Env *, Sv *);
extern Sv *Sv_call(struct Stu *, struct Env *, Sv *, Sv *);

#endif
