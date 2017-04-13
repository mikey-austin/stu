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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "svlist.h"
#include "builtins.h"
#include "stu_private.h"
#include "sv.h"
#include "native_func.h"

typedef enum {
  INTEGER,
  RATIONAL,
  REAL
} value_type;

#define DEF(name, func, nargs, rest) Sv_native_func_register((stu), (name), (func), (nargs), (rest))
extern void
Builtin_init(Stu *stu)
{
    DEF("defmacro", Builtin_defmacro, 3, 1 );
    DEF("+", Builtin_add, 1, 1);
    DEF("-", Builtin_sub, 1, 1);
    DEF("*", Builtin_mul, 1, 1);
    DEF("/", Builtin_div, 1, 1);
    DEF("quote", Builtin_quote, 1, 0);
    DEF("def", Builtin_def, 2, 0);
    DEF("cons", Builtin_cons, 2, 0);
    DEF("list", Builtin_list, 1, 1);
    DEF("\xce\xbb", Builtin_lambda, 2, 1);
    DEF("lambda", Builtin_lambda, 2, 1);
    DEF("macroexpand-1", Builtin_macroexpand_1, 1, 0);
    DEF("macroexpand", Builtin_macroexpand, 1, 0);
    DEF("progn", Builtin_progn, 1, 0);
    DEF("eval", Builtin_eval, 1, 0);
    DEF("car", Builtin_car, 1, 0);
    DEF("cdr", Builtin_cdr, 1, 0);
    DEF("reverse", Builtin_reverse, 1, 0);
    DEF("if", Builtin_if, 3, 1);
    DEF("read", Builtin_read, 1, 0);
    DEF("print", Builtin_print, 1, 0);
    DEF("=", Builtin_eq, 1, 1);
    DEF(">", Builtin_gt, 1, 1);
    DEF("<", Builtin_lt, 1, 1);
    DEF(">=", Builtin_gte, 1, 1);
    DEF("<=", Builtin_lte, 1, 1);
    DEF("vector", Builtin_vector, 1, 1);
    DEF("tuple-constructor", Builtin_tuple_constructor, 2, 0);
    DEF("size", Builtin_size, 1, 0);
    DEF("at", Builtin_at, 2, 0);
}
#undef DEF

#define INIT_ACC(init) value_type acc_type = INTEGER, cur_type = INTEGER; \
                       Sv_rational racc, r; \
                       double facc = (init), f; \
                       long acc = (init), i; \
                       racc.n = 0; racc.d = 0;

#define SET_ACC(op) switch (cur->type) { \
                    case SV_INT: \
                        cur_type = INTEGER; \
                        i = cur->val.i; \
                        break; \
                    case SV_RATIONAL: \
                        if (acc_type == REAL) { \
                            cur_type = REAL; \
                            f = (float) cur->val.rational.n / cur->val.rational.d; \
                        } else { \
                            if (cur_type != RATIONAL) { \
                                racc.n = acc; \
                                racc.d = acc; \
                            } \
                            acc_type = RATIONAL; \
                            cur_type = RATIONAL; \
                            r.n = cur->val.rational.n; \
                            r.d = cur->val.rational.d; \
                        } \
                        break; \
                    case SV_FLOAT: \
                        if (acc_type == RATIONAL) { \
                            facc = (float) racc.n / racc.d; \
                            acc_type = REAL; \
                        } \
                        if (acc_type != REAL) \
                            facc = acc; \
                        cur_type = REAL; \
                        acc_type = REAL; \
                        f = cur->val.f; \
                        break; \
                    default: \
                        return Sv_new_err(stu, op " can operate on numbers only"); \
                    }

#define RET_ACC return acc_type == RATIONAL \
                    ? Sv_new_rational(stu, racc.n, racc.d) \
                    : acc_type == REAL \
                        ? Sv_new_float(stu, facc) \
                        : Sv_new_int(stu, acc); \

extern Sv
*Builtin_add(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    Sv *cur = NULL;
    INIT_ACC(0);

    while (!IS_NIL(x) && (cur = CAR(x))) {
        SET_ACC("+");
        if (acc_type == RATIONAL) {
            if (cur_type != RATIONAL) {
              r.n = i;
              r.d = 1;
            }

            if (racc.n == 0) {
                racc.n = r.n;
                racc.d = r.d;
            } else if (r.d == racc.d) {
                racc.n += r.n;
                racc.d = r.d;
            } else {
                racc.n *= r.d;
                r.n *= racc.d;
                r.d *= racc.d;
                racc.d = r.d;
                racc.n += r.n;
            }
        } else if (acc_type == REAL)
            facc += (cur_type == REAL ? f : i);
        else
            acc += i;
        x = CDR(x);
    }

    RET_ACC;
}

extern Sv
*Builtin_sub(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    Sv *cur = NULL;
    INIT_ACC(0);

    if (!IS_NIL(x) && (cur = CAR(x))) {
        SET_ACC("-");
        if (acc_type == RATIONAL) {
            racc.n = r.n;
            racc.d = r.d;
        } else if (acc_type == REAL)
            facc = (cur_type == REAL ? f : i);
        else
            acc = i;
        x = CDR(x);

        if (!IS_NIL(x)) {
            while (!IS_NIL(x) && (cur = CAR(x))) {
                SET_ACC("-");
                if (acc_type == RATIONAL) {
                    /*
                     * If the current value is not a float, it's for sure an
                     * integer since we reset the rational flag otherwise.
                     */
                    if (cur_type != RATIONAL) {
                      r.n = i;
                      r.d = 1;
                    }

                    if (r.d == racc.d) {
                        racc.n -= r.n;
                        racc.d = r.d;
                    } else {
                        racc.n *= r.d;
                        r.n *= racc.d;
                        r.d *= racc.d;
                        racc.d = r.d;
                        racc.n -= r.n;
                    }
                } else if (acc_type == REAL)
                    facc -= (cur_type == REAL ? f : i);
                else
                    acc -= i;
                x = CDR(x);
            }
        } else {
          if (acc_type == RATIONAL) {
                racc.n = -racc.n;
          } else if (acc_type == REAL)
                facc = -facc;
            else
                acc = -acc;
        }
    }

    RET_ACC;
}

extern Sv
*Builtin_mul(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    Sv *cur = NULL;
    INIT_ACC(1);
    racc.n = 1;
    racc.d = 1;

    while (!IS_NIL(x) && (cur = CAR(x))) {
        SET_ACC("*");
        if (acc_type == RATIONAL) {
            if (acc != 1) {
              racc.n = acc;
              racc.d = 1;
            }

            /*
             * If the current value is not a float, it's for sure an
             * integer since we reset the rational flag otherwise.
             */
            if (cur_type != RATIONAL) {
                racc.n *= i;
            } else {
                racc.n *= r.n;
                racc.d *= r.d;
            }
        } else if (acc_type == REAL)
            facc *= (cur_type == REAL ? f : i);
        else
            acc *= i;
        x = CDR(x);
    }

    RET_ACC;
}

extern Sv
*Builtin_div(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    Sv *cur = NULL;
    INIT_ACC(0);

    if (!IS_NIL(x) && (cur = CAR(x))) {
        SET_ACC("/");
        if (acc_type == RATIONAL) {
            racc.n = r.n;
            racc.d = r.d;
        } else if (acc_type == REAL)
            facc = (cur_type == REAL ? f : i);
        else
            acc = i;
        x = CDR(x);

        if (!IS_NIL(x)) {
            while (!IS_NIL(x) && (cur = CAR(x))) {
                SET_ACC("/");
                if (cur_type == REAL && f == 0)
                    return Sv_new_err(stu, "'/' cannot divide by zero!");
                if (cur_type != RATIONAL && i == 0)
                    return Sv_new_err(stu, "'/' cannot divide by zero!");

                if (acc_type == RATIONAL) {
                  if (cur_type == RATIONAL) {
                    racc.n *= r.d;
                    racc.d *= r.n;
                  } else {
                    racc.d *= i;
                  }
                } else if (acc_type == REAL)
                    facc /= (cur_type == REAL ? f : i);
                else
                    acc /= i;
                x = CDR(x);
            }
        } else {
            if (acc_type == REAL)
                facc = 1 / (cur_type == REAL ? f : i);
            else {
                acc_type = RATIONAL;
                racc.n = 1;
                racc.d = i;
            }
        }
    } else {
        return Sv_new_err(stu, "'/' requires one or more arguments");
    }

    RET_ACC;
}

extern Sv
*Builtin_quote(Stu *stu, Env *env, Sv **args)
{
    return *args;
}

extern Sv
*Builtin_def(Stu *stu, Env *env, Sv **args)
{
    Sv *y = args[0];
    Sv *z = args[1];

    if (y && y->type != SV_SYM)
        y = Sv_eval(stu, env, y);

    /* We need a symbol in the head. */
    if (!y || y->type != SV_SYM)
        return Sv_new_err(stu, "'def' needs a symbol as the first argument");

    /* Def in the top scope. */
    z = Sv_eval(stu, env, z);
    Env_main_put(stu, y, z);

    /*
     * If we installed a lambda, also install in the lambda's env so
     * it can call itself.
     */
    if (z->type == SV_LAMBDA)
        z->val.ufunc->env = Env_put(stu, z->val.ufunc->env, y, z);

    return NIL;
}

extern Sv
*Builtin_cons(Stu *stu, Env *env, Sv **args)
{
    return Sv_cons(stu, args[0], args[1]);
}

extern Sv
*Builtin_list(Stu *stu, Env *env, Sv **x)
{
    return Sv_list(stu, *x);
}

extern Sv
*Builtin_lambda(Stu *stu, Env *env, Sv **args)
{
    Sv *formals = NULL, *cur = NULL;

    /* All formals should be symbols. */
    formals = args[0];
    if (formals->type == SV_CONS || IS_NIL(formals)) {
        while (!IS_NIL(formals) && formals->type == SV_CONS && (cur = CAR(formals))) {
            if (cur->type != SV_SYM) {
                return Sv_new_err(
                    stu, "'lambda' formals need to be symbols");
            }
            formals = CDR(formals);
        }
    } else {
        return Sv_new_err(
            stu, "'lambda' needs a list of symbols as the first argument");
    }

    formals = args[0];
    cur = args[1];

    return Sv_new_lambda(stu, env, formals, cur);
}

extern Sv
*Builtin_defmacro(Stu *stu, Env *env, Sv **args)
{
    Sv *name = args[0];
    Sv *lambda = Builtin_lambda(stu, env, args + 1);

    lambda->val.ufunc->is_macro = 1;
    Env_main_put(stu, name, lambda);

    return name;
}

extern Sv
*Builtin_macroexpand_1(Stu *stu, Env *env, Sv **x)
{
    return Sv_expand_1(stu, *x);
}

extern Sv
*Builtin_macroexpand(Stu *stu, Env *env, Sv **x)
{
    return Sv_expand(stu, *x);
}

extern Sv
*Builtin_progn(Stu *stu, Env *env, Sv **x)
{
    return Sv_eval_list(stu, env, *x);
}

extern Sv
*Builtin_eval(Stu *stu, Env *env, Sv **x)
{
    return Sv_eval(stu, env, *x);
}

extern Sv
*Builtin_car(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    if (x->type != SV_CONS)
        return Sv_new_err(stu, "'car' needs a single list argument");
    return CAR(x);
}

extern Sv
*Builtin_cdr(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    if (x->type != SV_CONS)
        return Sv_new_err(stu, "'cdr' needs a single list argument");
    return CDR(x);
}

extern Sv
*Builtin_reverse(Stu *stu, Env *env, Sv **args)
{
    Sv *x = *args;
    if (x->type != SV_CONS)
        return Sv_new_err(stu, "'reverse' needs a single list argument");
    return Sv_reverse(stu, x);
}

extern Sv
*Builtin_if(Stu *stu, Env *env, Sv **args)
{
    Sv *cond = args[0], *first = args[1], *second = args[2];

    cond = Sv_eval(stu, env, cond);
    if (!cond || cond->type != SV_BOOL)
        return Sv_new_err(stu, "'if' condition must evaluate to a bool");

    if (cond->val.i) {
        return Sv_eval(stu, env, first);
    } else {
        return IS_NIL(second) ? second : Sv_eval(stu, env, CAR(second));
    }
}

extern Sv
*Builtin_read(Stu *stu, Env *env, Sv **x)
{
    Sv *code = *x, *result = NULL;
    Svlist *forms = NULL;

    if (code->type != SV_STR || code->val.buf == NULL)
        return Sv_new_err(stu, "read expects a string argument");

    forms = Stu_parse_buf(stu, code->val.buf);
    if (forms->count != 1) {
        result = Sv_new_err(stu, "read argument must contain exactly one form");
    } else {
        result = forms->head->sv;
    }

    Svlist_destroy(&forms);

    return result;
}

extern Sv
*Builtin_print(Stu *stu, Env *env, Sv **x)
{
    Sv_dump(stu, *x, stdout);
    return *x;
}

/*
 * Generate comparison functions using a macro.
 */

#define OP_EQ  1
#define OP_GT  2
#define OP_LT  3
#define OP_GTE 4
#define OP_LTE 5

#define CMP_SV(op) \
    Sv *sv = *args; \
    Sv *x = CAR(sv), *y = CADR(sv), *rest = CDR(sv); \
    short result = 1; \
    if (!x && !y) return Sv_new_bool(stu, 1);    \
    if (x && y) { \
        while (!IS_NIL(rest) && (y = CAR(rest))) { \
            if (x->type == y->type) { \
                switch (x->type) { \
                    case SV_NIL: \
                        result = 1; \
                        break; \
                    case SV_INT: \
                    case SV_BOOL: \
                    case SV_SYM: \
                        result = result && compare_numbers(op, x->val.i, y->val.i); \
                        break; \
                    case SV_RATIONAL: \
                        result = result && compare_rationals(op, x->val.rational, y->val.rational); \
                        break; \
                    case SV_STR: \
                    case SV_ERR: \
                        result = result && compare_strings(op, x->val.buf, y->val.buf); \
                        break; \
                    default: \
                        return Sv_new_err(stu, "'eq' does not support these types"); \
                } \
            } else if (x->type == SV_INT && x->type == SV_RATIONAL) { \
                result = result && compare_numbers(op, x->val.i * y->val.rational.d, y->val.rational.n); \
            } else if (y->type == SV_INT && x->type == SV_RATIONAL) { \
                result = result && compare_numbers(op, x->val.rational.n, y->val.i * x->val.rational.d); \
            } else { \
                return Sv_new_bool(stu, 0);      \
            } \
            \
            if (!result) return Sv_new_bool(stu, 0); \
            \
            x = y; rest = CDR(rest); \
        } \
        \
        if (IS_NIL(rest)) { \
            return Sv_new_bool(stu, result);     \
        } \
    } \
    return Sv_new_bool(stu, 0);

static int
compare_numbers(int op, long x, long y)
{
    switch (op) {
    case OP_EQ:
        return x == y ? 1 : 0;

    case OP_GT:
        return x > y ? 1 : 0;

    case OP_LT:
        return x < y ? 1 : 0;

    case OP_GTE:
        return x >= y ? 1 : 0;

    case OP_LTE:
        return x <= y ? 1 : 0;

    default:
        return 0;
    }
}

static int
compare_rationals(int op, Sv_rational x, Sv_rational y)
{
    return compare_numbers(op, x.n * y.d, x.d * y.n);
}

static int
compare_strings(int op, const char *x, const char *y)
{
    switch (op) {
    case OP_EQ:
        return !strcmp(x, y) ? 1 : 0;

    case OP_GT:
        return strcmp(x, y) > 0 ? 1 : 0;

    case OP_LT:
        return strcmp(x, y) < 0 ? 1 : 0;

    case OP_GTE:
        return strcmp(x, y) >= 0 ? 1 : 0;

    case OP_LTE:
        return strcmp(x, y) <= 0 ? 1 : 0;

    default:
        return 0;
    }
}

extern Sv
*Builtin_eq(Stu *stu, Env *env, Sv **args)
{
    CMP_SV(OP_EQ);
}

extern Sv
*Builtin_gt(Stu *stu, Env *env, Sv **args)
{
    CMP_SV(OP_GT);
}

extern Sv
*Builtin_lt(Stu *stu, Env *env, Sv **args)
{
    CMP_SV(OP_LT);
}

extern Sv
*Builtin_gte(Stu *stu, Env *env, Sv **args)
{
    CMP_SV(OP_GTE);
}

extern Sv
*Builtin_lte(Stu *stu, Env *env, Sv **args)
{
    CMP_SV(OP_LTE);
}

extern Sv
*Builtin_vector(Stu *stu, Env *env, Sv **args)
{
    return Sv_new_vector(stu, args[0]);
}

extern Sv
*Builtin_tuple_constructor(Stu *stu, Env *env, Sv **args)
{
    Sv *name = args[0];
    Sv *arity = args[1];

    if (name->type != SV_SYM || arity->type != SV_INT)
        return NULL;

    long num = arity->val.i;
    if (num < 0)
        return NULL;

    Sv_tuple_type *tt = malloc(sizeof(*tt));
    if (tt == NULL)
        return NULL;
    tt->name = name->val.i;
    tt->arity = num;

    return Sv_new_tuple_constructor(stu, tt);
}

extern Sv
*Builtin_size(Stu *stu, Env *env, Sv **args)
{
    Sv *tuple = args[0];
    if (tuple->type != SV_TUPLE)
        return NULL;
    return Sv_new_int(stu, tuple->val.tuple->type->arity);
}

extern Sv
*Builtin_at(Stu *stu, Env *env, Sv **args)
{
    Sv *index = args[0];
    Sv *tuple_sv = args[1];
    if (index->type != SV_INT)
        return NULL;
    if (tuple_sv->type != SV_TUPLE)
        return NULL;
    Sv_tuple *tuple = tuple_sv->val.tuple;
    long i = index->val.i;
    if (i < 0 || i > tuple->type->arity)
        return NULL;
    return tuple->values[i];

}
