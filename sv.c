#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#include "sv.h"
#include "env.h"

static void Sv_cons_dump(Sv *);

extern Sv
*Sv_new(enum Sv_type type)
{
    Sv *x = NULL;
    int i;

    if ((x = malloc(sizeof(*x))) != NULL) {
        x->type = type;
        x->special = 0;
        for (i = 0; i < SV_CONS_REGISTERS; i++)
            x->val.reg[i] = NULL;
    } else {
        err(1, "Sv_new");
    }

    return x;
}

extern Sv
*Sv_new_int(long i)
{
    Sv *x = Sv_new(SV_INT);
    x->val.i = i;
    return  x;
}

extern Sv
*Sv_new_bool(short i)
{
    Sv *x = Sv_new(SV_BOOL);
    x->val.i = (int) i;
    return  x;
}

extern Sv
*Sv_new_str(const char *str)
{
    Sv *x = Sv_new(SV_STR);
    if ((x->val.buf = strdup(str)) == NULL)
        err(1, "Sv_new_str");
    return  x;
}

extern Sv
*Sv_new_sym(const char *sym)
{
    Sv *x = Sv_new_str(sym);
    x->type = SV_SYM;
    x->special = !strcmp(sym, "quote")
        || !strcmp(sym, "if")
        || !strcmp(sym, "lambda");
    return x;
}

extern Sv
*Sv_new_err(const char *err)
{
    Sv *x = Sv_new_str(err);
    x->type = SV_ERR;
    return x;
}

extern Sv
*Sv_new_func(Sv_func func)
{
    Sv *x = Sv_new(SV_FUNC);
    x->val.func = func;
    return x;
}

extern Sv
*Sv_new_lambda(Sv *formals, Sv *body)
{
    Sv *x = Sv_new(SV_LAMBDA);
    Sv_ufunc *f = NULL;

    if ((f = malloc(sizeof(*f))) != NULL) {
        f->env = Env_new();
        f->formals = formals;
        f->body = body;
        x->val.ufunc = f;
    } else {
        err(1, "Sv_new_lambda");
    }

    return x;
}

extern void
Sv_destroy(Sv **sv)
{
    if (sv && *sv) {
        switch ((*sv)->type) {
        case SV_ERR:
        case SV_SYM:
        case SV_STR:
            if ((*sv)->val.buf) {
                free((*sv)->val.buf);
                (*sv)->val.buf = NULL;
            }
            break;

        case SV_CONS:
            for (int i = 0; i < SV_CONS_REGISTERS; i++) {
                Sv_destroy(&((*sv)->val.reg[i]));
                (*sv)->val.reg[i] = NULL;
            }
            break;

        case SV_FUNC:
        case SV_BOOL:
        case SV_INT:
            break;

        case SV_LAMBDA:
            if ((*sv)->val.ufunc) {
                Env_destroy(&((*sv)->val.ufunc->env));
                Sv_destroy(&((*sv)->val.ufunc->formals));
                Sv_destroy(&((*sv)->val.ufunc->body));
                free((*sv)->val.ufunc);
                (*sv)->val.ufunc = NULL;
            }
            break;
        }

        free(*sv);
        *sv = NULL;
    }
}

extern void
Sv_dump(Sv *sv)
{
    if (sv) {
        switch (sv->type) {
        case SV_SYM:
        case SV_ERR:
        case SV_STR:
            if (sv->val.buf)
                printf("%s", sv->val.buf);
            break;

        case SV_CONS:
            printf("(");
            Sv_cons_dump(sv);
            printf(")");
            break;

        case SV_INT:
            printf("%ld", sv->val.i);
            break;

        case SV_BOOL:
            printf("%s", sv->val.i ? "#t" : "#f");
            break;

        case SV_FUNC:
            printf("<builtin>");
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                printf("(lambda ");
                Sv_dump(sv->val.ufunc->formals);
                putchar(' ');
                Sv_dump(sv->val.ufunc->body);
                putchar(')');
            }
            break;
        }
    } else {
        printf("nil");
    }
}

static void
Sv_cons_dump(Sv *sv)
{
    Sv *car = CAR(sv);
    Sv *cdr = CDR(sv);
    Sv_dump(car);
    if (cdr) {
        if (cdr->type == SV_CONS) {
            printf(" ");
            Sv_cons_dump(cdr);
        } else {
            printf(" . ");
            Sv_dump(cdr);
        }
    }
}

extern Sv
*Sv_cons(Sv *x, Sv *y)
{
    Sv *z = Sv_new(SV_CONS);
    if (z != NULL) {
        z->val.reg[SV_CAR_REG] = x;
        z->val.reg[SV_CDR_REG] = y;
    } else {
        err(1, "Sv_cons");
    }

    return z;
}

extern Sv
*Sv_reverse(Sv *x)
{
    Sv *y = NULL;

    while (x && x->type == SV_CONS) {
        y = Sv_cons(CAR(x), y);
        x = CDR(x);
    }

    return y;
}

extern Sv
*Sv_eval(Env *env, Sv *x)
{
    Sv *y = NULL;

    if (!x)
        return x;

    switch (x->type) {
    case SV_SYM:
        if ((y = Env_get(env, x)) == NULL) {
            y = Sv_new_err("possibly unknown symbol");
        }
        return y;

    case SV_CONS:
        return Sv_eval_sexp(env, x);

    default:
        return x;
    }
}

extern Sv
*Sv_eval_sexp(Env *env, Sv *x)
{
    Sv *cur = x, *y = NULL, *z = NULL;

    if (!cur)
        return NULL;

    z = CAR(x);
    if (z && z->special) {
        /* Only evaluate the head, leaving tail intact. */
        y = Sv_eval(env, z);
    } else {
        /* Evaluate all arguments. */
        do {
            if (cur->type == SV_CONS) {
                y = Sv_cons(Sv_eval(env, CAR(cur)), y);
                cur = CDR(cur);
            } else {
                y = Sv_cons(cur, y);
                break;
            }
        } while (cur);
        x = Sv_reverse(y);
        y = CAR(x);
    }

    /* The car should now be a function. */
    if (y->type == SV_FUNC || y->type == SV_LAMBDA) {
        return Sv_call(env, y, CDR(x));
    }

    return Sv_new_err("first element is not a function");
}

extern Sv
*Sv_call(struct Env *env, Sv *f, Sv *a)
{
    Sv *formals, *formal, *arg, *args = a;
    formals = formal = arg = NULL;

    if (!f)
        return f;

    if (f->type == SV_FUNC)
        return f->val.func(env, a);

    if (f->type == SV_LAMBDA) {
        /* Bind the formals to the arguments. */
        formals = f->val.ufunc->formals;

        while (formals && (formal = CAR(formals))) {
            /* Get the next arg. */
            if (args && (arg = CAR(args))) {
                Env_put(env, formal, arg);
            } else {
                return Sv_new_err("missing some arguments");
            }

            formals = CDR(formals);
            args = CDR(args);
        }

        return Sv_eval(env, f->val.ufunc->body);
    }

    return Sv_new_err("can only call functions");
}
