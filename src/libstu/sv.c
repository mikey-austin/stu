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

#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#include "alloc/alloc.h"
#include "env.h"
#include "gc.h"
#include "native_func.h"
#include "stu_private.h"
#include "sv.h"
#include "symtab.h"
#include "utils.h"

extern Sv
*Sv_new(Stu *stu, enum Sv_type type)
{
    Sv *x = NULL;

    if ((x = Alloc_allocate(stu->sv_alloc)) != NULL) {
        GC_INIT(stu, x, GC_TYPE_SV);
        x->type = type;
    } else {
        err(1, "Sv_new");
    }

    return x;
}

extern Sv
*Sv_new_int(Stu *stu, long i)
{
    Sv *x = Sv_new(stu, SV_INT);
    x->val.i = i;
    return x;
}

extern Sv
*Sv_new_float(Stu *stu, double f)
{
    Sv *x = Sv_new(stu, SV_FLOAT);
    x->val.f = f;
    return x;
}

extern Sv
*Sv_new_rational(Stu *stu, long n, long d)
{
    if (n % d == 0)
        return Sv_new_int(stu, n / d);

    Sv *x = Sv_new(stu, SV_RATIONAL);
    long max_search = labs(n) > d ? d : labs(n);
    long cur = 2;

    while (cur <= max_search) {
        if ((n % cur == 0) && (d % cur == 0)) {
            n /= cur;
            d /= cur;
        } else {
            cur++;
        }
    }

    x->val.rational.n = n;
    x->val.rational.d = d;

    return x;
}

extern Sv
*Sv_new_bool(Stu *stu, short i)
{
    Sv *x = Sv_new(stu, SV_BOOL);
    x->val.i = (int) i;
    return x;
}

extern Sv
*Sv_new_str(Stu *stu, const char *str)
{
    Sv *x = Sv_new(stu, SV_STR);
    if ((x->val.buf = strdup(str)) == NULL)
        err(1, "Sv_new_str");
    return x;
}

extern Sv
*Sv_new_sym(Stu *stu, const char *sym)
{
    Sv *x = Sv_new(stu, SV_SYM);

    x->special = !strcmp(sym, "quote")
        || !strcmp(sym, "def")
        || !strcmp(sym, "defmacro")
        || !strcmp(sym, "lambda")
        || !strcmp(sym, "λ")
        || !strcmp(sym, "if");
    x->val.i = Symtab_get_id(stu, sym);

    return x;
}

extern Sv
*Sv_new_err(Stu *stu, const char *err)
{
    Sv *x = Sv_new_str(stu, err);
    x->type = SV_ERR;
    return x;
}

extern Sv
*Sv_new_native_func(Stu *stu, Sv_native_func_t f, unsigned arity, unsigned rest)
{
    Sv *x = Sv_new(stu, SV_NATIVE_FUNC);
    x->val.func = Sv_native_func_new(stu, f, arity, rest);
    return x;
}

extern Sv
*Sv_new_special(Stu *stu, enum Sv_special_type type, Sv *body)
{
    Sv *x = Sv_new(stu, SV_SPECIAL);
    Sv_special *s = CHECKED_MALLOC(sizeof(*s));

    s->type = type;
    s->body = Sv_copy(stu, body);
    x->val.special = s;

    return x;
}

extern Sv
*Sv_new_lambda(Stu *stu, Env *env, Sv *formals, Sv *body)
{
    Sv *x = Sv_new(stu, SV_LAMBDA);
    Sv_ufunc *f = CHECKED_MALLOC(sizeof(*f));

    f->env = env;
    f->formals = formals;
    f->body = body;
    f->is_macro = 0;
    x->val.ufunc = f;

    return x;
}

extern Sv
*Sv_new_vector(struct Stu *stu, Sv *sv)
{
    unsigned count = 0;
    for (Sv *tmp = sv; !IS_NIL(tmp); tmp = CDR(tmp))
        if (tmp->type == SV_CONS)
            ++count;
        else
            return Sv_new_err(stu, "Vector argument is not a proper list");

    Sv_tuple_type *type = CHECKED_MALLOC(sizeof *type);

    type->name = Sv_new_sym(stu, "vector")->val.i;
    type->arity = count;

    return Sv_new_tuple(stu, type, sv);

}

extern Sv
*Sv_new_tuple(struct Stu *stu, Sv_tuple_type *type, Sv *args) {
    Sv *x = Sv_new(stu, SV_TUPLE);
    Sv_tuple *tup = CHECKED_MALLOC(sizeof(*tup) + type->arity * sizeof(Sv*));

    unsigned i = 0;
    while (!IS_NIL(args) && args->type == SV_CONS && i < type->arity) {
        tup->values[i++] = CAR(args);
        args = CDR(args);
    }

    if (!IS_NIL(args) || i != type->arity)
        return Sv_new_err(stu, "List length doesn't match tuple arity");

    tup->type = type;
    x->val.tuple = tup;

    return x;
}

extern Sv
*Sv_new_tuple_constructor(struct Stu *stu, Sv_tuple_type *tuple_type) {
    Sv *x = Sv_new(stu, SV_TUPLE_CONSTRUCTOR);
    x->val.tuple_constructor = tuple_type;
    return x;
}


extern void
Sv_destroy(Stu *stu, Sv **sv)
{
    int i;

    if (sv && *sv) {
        switch ((*sv)->type) {
        case SV_ERR:
        case SV_STR:
            if ((*sv)->val.buf) {
                free((*sv)->val.buf);
                (*sv)->val.buf = NULL;
            }
            break;

        case SV_CONS:
            /* GC will clean up cons cells. */
            for (i = 0; i < SV_CONS_REGISTERS; i++)
                (*sv)->val.reg[i] = NULL;
            break;

        case SV_LAMBDA:
            /* GC will clean up environments and lists. */
            if ((*sv)->val.ufunc) {
                (*sv)->val.ufunc->env = NULL;
                (*sv)->val.ufunc->formals = NULL;
                (*sv)->val.ufunc->body = NULL;
                free((*sv)->val.ufunc);
                (*sv)->val.ufunc = NULL;
            }
            break;

        case SV_SPECIAL:
            /* GC will clean up special body. */
            if ((*sv)->val.special) {
                (*sv)->val.special->body = NULL;
                free((*sv)->val.special);
                (*sv)->val.special = NULL;
            }
            break;

        case SV_NATIVE_FUNC:
            free((*sv)->val.func);
            (*sv)->val.func = NULL;
            break;

        case SV_NATIVE_CLOS:
            free((*sv)->val.clos);
            (*sv)->val.clos = NULL;
            break;

        default:
            break;
        }

        Alloc_release(stu->env_alloc, *sv);
        *sv = NULL;
    }
}

static void
Sv_cons_dump(Stu *stu, Sv *sv, FILE *out)
{
    Sv *car = CAR(sv);
    Sv *cdr = CDR(sv);
    Sv_dump(stu, car, out);
    if (cdr) {
        switch (cdr->type) {
        case SV_CONS:
            printf(" ");
            Sv_cons_dump(stu, cdr, out);
            break;

        case SV_NIL:
            /* Don't print terminating nil. */
            break;

        default:
            printf(" . ");
            Sv_dump(stu, cdr, out);
            break;
        }
    } else {
        err(1, "NULL value in CDR");
    }
}

extern void
Sv_dump(Stu *stu, Sv *sv, FILE *out)
{
    if (sv) {
        switch (sv->type) {
        case SV_SYM:
            printf("%s", Symtab_get_name(stu, sv->val.i));
            break;

        case SV_ERR:
        case SV_STR:
            if (sv->val.buf)
                printf("\"%s\"", sv->val.buf);
            break;

        case SV_CONS:
            printf("(");
            Sv_cons_dump(stu, sv, out);
            printf(")");
            break;

        case SV_SPECIAL:
            switch (sv->val.special->type) {
                case SV_SPECIAL_COMMA:
                    printf(",");
                    break;
                case SV_SPECIAL_COMMA_SPREAD:
                    printf(",@");
                    break;
                case SV_SPECIAL_BACKQUOTE:
                    printf("`");
                    break;
            }

            Sv_dump(stu, sv->val.special->body, out);
            break;

        case SV_INT:
            printf("%ld", sv->val.i);
            break;

        case SV_FLOAT:
            printf("%0.10f", sv->val.f);
            break;

        case SV_RATIONAL:
            printf("%ld/%ld", sv->val.rational.n, sv->val.rational.d);
            break;

        case SV_BOOL:
            printf("%s", sv->val.i ? "#t" : "#f");
            break;

        case SV_NATIVE_FUNC:
            printf("<native-function>");
            break;

        case SV_NATIVE_CLOS:
            printf("<native-closure>");
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                printf("(λ ");
                Sv_dump(stu, sv->val.ufunc->formals, out);
                putchar(' ');
                Sv_dump(stu, sv->val.ufunc->body, out);
                putchar(')');
            }
            break;

        case SV_NIL:
            printf("nil");
            break;

        case SV_TUPLE_CONSTRUCTOR:
            printf("<TUPLE_CONSTRUCTOR %s %u>",
                   Symtab_get_name(stu, sv->val.tuple_constructor->name),
                   sv->val.tuple_constructor->arity);
            break;

        case SV_TUPLE:
            printf("[%s", Symtab_get_name(stu, sv->val.tuple->type->name));
            for (unsigned i = 0; i < sv->val.tuple->type->arity; ++i) {
                putchar(' ');
                Sv_dump(stu, sv->val.tuple->values[i], out);
            }
            puts("]");
            break;
        }
    }
}

static Sv
*Sv_copy_sym(Stu *stu, Sv *x)
{
    Sv *y = Sv_new(stu, SV_SYM);
    y->special = x->special;
    y->val.i = x->val.i;
    return y;
}

static Sv
*Sv_copy_tuple(Stu *stu, Sv *x)
{
    Sv_tuple *t = x->val.tuple;
    size_t size = sizeof(Sv_tuple) + t->type->arity * sizeof(Sv*);
    Sv_tuple *t_copy = malloc(size);
    if (t_copy == NULL)
        err(1, "Sv_copy_tuple");
    memcpy(t_copy, t, size);
    Sv *copy = Sv_new(stu, SV_TUPLE);
    copy->val.tuple =  t_copy;
    return copy;
}


extern Sv
*Sv_copy(Stu *stu, Sv *x)
{
    int i;
    Sv *y = NULL;

    if (x) {
        switch (x->type) {
        case SV_NIL:
            y = x;
            break;

        case SV_SYM:
            y = Sv_copy_sym(stu, x);
            break;

        case SV_ERR:
            if (x->val.buf)
                y = Sv_new_err(stu, x->val.buf);
            break;

        case SV_STR:
            if (x->val.buf)
                y = Sv_new_str(stu, x->val.buf);
            break;

        case SV_CONS:
            y = Sv_new(stu, SV_CONS);
            for (i = 0; i < SV_CONS_REGISTERS; i++) {
                y->val.reg[i] = Sv_copy(stu, x->val.reg[i]);
            }
            break;

        case SV_SPECIAL:
            y = Sv_new_special(stu, x->val.special->type, x->val.special->body);
            break;

        case SV_INT:
            y = Sv_new_int(stu, x->val.i);
            break;

        case SV_FLOAT:
            y = Sv_new_float(stu, x->val.f);
            break;

        case SV_RATIONAL:
            y = Sv_new_rational(stu, x->val.rational.n, x->val.rational.d);
            break;

        case SV_BOOL:
            y = Sv_new_bool(stu, (short) x->val.i);
            break;

        case SV_LAMBDA:
            if (x->val.ufunc) {
                y = Sv_new_lambda(
                    stu, x->val.ufunc->env, x->val.ufunc->formals, x->val.ufunc->body);
            }
            break;

        case SV_TUPLE:
            y = Sv_copy_tuple(stu, x);
            break;
        }
    }

    return y;
}

extern Sv
*Sv_cons(Stu *stu, Sv *x, Sv *y)
{
    Sv *z = Sv_new(stu, SV_CONS);

    z->val.reg[SV_CAR_REG] = x;
    z->val.reg[SV_CDR_REG] = y;

    return z;
}

extern Sv
*Sv_reverse(Stu *stu, Sv *x)
{
    Sv *y = NIL;

    while (!IS_NIL(x) && x->type == SV_CONS) {
        y = Sv_cons(stu, CAR(x), y);
        x = CDR(x);
    }

    return y;
}

extern Sv
*Sv_list(Stu *stu, Sv *x)
{
    Sv *y = NULL, *z = NIL;

    if (!x)
        return x;

    if (x->type != SV_CONS)
        return Sv_cons(stu, x, NIL);

    while (!IS_NIL(x) && (y = CAR(x))) {
        z = Sv_cons(stu, y, z);
        x = CDR(x);
    }

    return Sv_reverse(stu, z);
}

extern Sv
*Sv_expand_1(Stu *stu, Sv *x)
{
    if (!x || x->type != SV_CONS)
        return x;

    Sv *head = CAR(x);
    Sv *macro;

    if (head->type != SV_SYM)
        return x;

    macro = Env_main_get(stu, head);
    if (!IS_MACRO(macro))
        return x;

    return Sv_call(stu, stu->main_env, macro, CDR(x));
}

extern Sv
*Sv_expand(Stu *stu, Sv *x)
{
    Sv *head;

    do {
        x = Sv_expand_1(stu, x);
        head = CAR(x);
    } while (IS_MACRO(Env_main_get(stu, head)));

    return x;
}

static Sv
*Sv_eval_tuple(Stu *stu, Env *env, Sv *x)
{
    Sv *copy = Sv_copy(stu, x);
    Sv_tuple *t = copy->val.tuple;
    unsigned size = t->type->arity;
    for (unsigned i = 0; i < size; ++i)
        t->values[i] = Sv_eval(stu, env, t->values[i]);
    return copy;
}

extern Sv
*Sv_eval(Stu *stu, Env *env, Sv *x)
{
    Sv *y = NULL;

    if (!x)
        return x;

    PUSH_SCOPE(stu);
    switch (x->type) {
    case SV_SYM:
        /*
         * If the symbol exists but it's value is NULL, then it is
         * the empty list.
         */
        if ((y = Env_get(env, x)) == NULL && !Env_exists(env, x)) {
            y = Sv_new_err(stu, "possibly unknown symbol");
        }
        break;

    case SV_SPECIAL:
        y = Sv_eval_special(stu, env, x);
        break;

    case SV_CONS:
        y = Sv_eval_sexp(stu, env, x);
        break;

    case SV_TUPLE:
        y = Sv_eval_tuple(stu, env, x);
        break;

    default:
        y = x;
        break;
    }
    POP_N_SAVE(stu, y);

    return y;
}

extern Sv
*Sv_eval_list(Stu *stu, Env *env, Sv *x)
{
    if (IS_NIL(x)) return x;

    Sv *head = Sv_eval(stu, env, CAR(x));

    if (IS_NIL(CDR(x))) {
        return head;
    } else {
        return Sv_eval_list(stu, env, CDR(x));
    }
}

extern Sv
*Sv_eval_special(Stu *stu, Env *env, Sv *x)
{
    Sv_special *special = x->val.special;
    Sv *body = x->val.special->body;

    switch (special->type) {
    case SV_SPECIAL_BACKQUOTE:
        if (body->type == SV_SYM) {
            return Sv_eval_sexp(stu, env,
                Sv_cons(stu, Sv_new_sym(stu, "quote"), Sv_cons(stu, body, NIL)));
        } else if (body->type == SV_CONS) {
            return Sv_eval_special_cons(stu, env, body);
        } else {
            return body;
        }
        break;

    case SV_SPECIAL_COMMA:
        return Sv_new_err(stu, "Comma can be used only inside backquoted list");

    case SV_SPECIAL_COMMA_SPREAD:
        return Sv_new_err(stu, "Comma spread can be used only inside backquoted list");
    }

    return Sv_new_err(stu, "Troubles with quoting magic");
}

extern Sv
*Sv_eval_special_cons(Stu *stu, Env *env, Sv *x)
{
    if (IS_NIL(x))
        return x;

    Sv *head = CAR(x);

    if (head->type == SV_SPECIAL) {
        Sv_special *special = head->val.special;

        switch (special->type) {
        case SV_SPECIAL_COMMA:
            return Sv_cons(
                stu, Sv_eval(stu, env, special->body),
                Sv_eval_special_cons(stu, env, CDR(x)));

        case SV_SPECIAL_COMMA_SPREAD:
            if (special->body->type == SV_SYM || special->body->type == SV_CONS) {
                Sv *val = Sv_eval(stu, env, special->body);
                if (!IS_NIL(val) && val->type == SV_CONS) {
                    return val;
                } else {
                    return Sv_new_err(stu, "spread operator applied to atom");
                }
            } else {
                return Sv_new_err(stu, "spread operator can be applied only to symbol");
            }
            break;

        case SV_SPECIAL_BACKQUOTE:
            return Sv_new_err(stu, "backquote is not permitted inside of other backquote");
        }
    } else if (head->type == SV_CONS)  {
        return Sv_cons(
            stu, Sv_eval_special_cons(stu, env, head), Sv_eval_special_cons(stu, env, CDR(x)));
    } else {
        return Sv_cons(stu, head, Sv_eval_special_cons(stu, env, CDR(x)));
    }

    return Sv_new_err(stu, "Got confused inside of backquoted list");
}

extern Sv
*Sv_eval_sexp(Stu *stu, Env *env, Sv *x)
{
    Sv *cur = NULL, *y = NULL, *z = NULL;
    cur = x = Sv_expand(stu, x);

    if (!cur)
        return NULL;

    z = CAR(cur);

    if (z && z->special) {
        /* Only evaluate the head, leaving tail intact. */
        y = Sv_eval(stu, env, z);
    } else {
        /* Evaluate all arguments. */
         while (!IS_NIL(cur)) {
            if (cur->type == SV_CONS) {
                y = Sv_cons(stu, Sv_eval(stu, env, CAR(cur)), y);
                cur = CDR(cur);
            } else {
                y = Sv_cons(stu, cur, y);
                break;
            }
         }
         x = Sv_reverse(stu, y);
         y = CAR(x);
    }

    /* The car should now be a function. */
    if (y)
        switch (y->type) {
        case SV_NATIVE_FUNC:
        case SV_NATIVE_CLOS:
        case SV_LAMBDA:
        case SV_TUPLE_CONSTRUCTOR:
            return Sv_call(stu, env, y, CDR(x));
        }

    return Sv_new_err(stu, "first element is not a function");
}

extern Sv
*Sv_call(Stu *stu, Env *env, Sv *f, Sv *a)
{
    short varargs = 0;
    Env *call_env;
    Sv *formals, *formal, *arg, *partial, *args = a;
    formals = formal = arg = partial = NULL;

    if (!f)
        return f;

    if (f->type == SV_NATIVE_FUNC)
        return Sv_native_func_call(stu, env, f->val.func, a);

    if (f->type == SV_NATIVE_CLOS)
        return Sv_native_closure_call(stu, env, f->val.clos, a);

    if (f->type == SV_TUPLE_CONSTRUCTOR)
        return Sv_new_tuple(stu, f->val.tuple_constructor, a);

    if (f->type == SV_LAMBDA) {
        /* Bind the formals to the arguments. */
        formals = f->val.ufunc->formals;
        call_env = f->val.ufunc->env;

        while (!IS_NIL(formals) && (formal = CAR(formals))) {
            if (!varargs && formal->type == SV_SYM && formal->val.i == Symtab_get_id(stu, "&")) {
                varargs = 1;
            } else {
                /* Get the next arg. */
                if (args && (arg = CAR(args))) {
                    if (varargs) {
                        call_env = Env_put(stu, call_env, formal, Sv_list(stu, args));
                        break;
                    } else {
                        call_env = Env_put(stu, call_env, formal, arg);
                    }
                } else if (varargs) {
                    /* Variable args not supplied. */
                    call_env = Env_put(stu, call_env, formal, NULL);
                    break;
                } else {
                    /* No more arguments to consume. */
                    partial = Sv_copy(stu, f);
                    partial->val.ufunc->formals = formals;
                    partial->val.ufunc->env = call_env;
                    break;
                }
                args = CDR(args);
            }
            formals = CDR(formals);
        }

        if (partial) {
            return partial;
        } else {
            return Sv_eval_list(stu, call_env, f->val.ufunc->body);
        }
    }

    return Sv_new_err(stu, "can only call functions");
}
