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
        || !strcmp(sym, "def")
        || !strcmp(sym, "lambda")
        || !strcmp(sym, "if");
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
*Sv_new_lambda(Env *env, Sv *formals, Sv *body)
{
    Sv *x = Sv_new(SV_LAMBDA);
    Sv_ufunc *f = NULL;

    if ((f = malloc(sizeof(*f))) != NULL) {
        f->env = Env_new();
        if (env)
            Env_copy(env, f->env);
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
        printf("()");
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
*Sv_copy(Sv *x)
{
    Sv *y = NULL;

    if (x) {
        switch (x->type) {
        case SV_SYM:
            if (x->val.buf)
                y = Sv_new_sym(x->val.buf);
            break;

        case SV_ERR:
            if (x->val.buf)
                y = Sv_new_err(x->val.buf);
            break;

        case SV_STR:
            if (x->val.buf)
                y = Sv_new_str(x->val.buf);
            break;

        case SV_CONS:
            y = Sv_new(SV_CONS);
            for (int i = 0; i < SV_CONS_REGISTERS; i++) {
                y->val.reg[i] = Sv_copy(x->val.reg[i]);
            }
            break;

        case SV_INT:
            y = Sv_new_int(x->val.i);
            break;

        case SV_BOOL:
            y = Sv_new_bool((short) x->val.i);
            break;

        case SV_FUNC:
            y = Sv_new_func(x->val.func);
            break;

        case SV_LAMBDA:
            if (x->val.ufunc) {
                y = Sv_new_lambda(x->val.ufunc->env, Sv_copy(x->val.ufunc->formals),
                    Sv_copy(x->val.ufunc->body));
            }
            break;
        }
    }

    return y;
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
*Sv_list(Sv *x)
{
    Sv *y = NULL, *z = NULL;

    if (!x)
        return x;

    if (x->type != SV_CONS)
        return Sv_cons(x, NULL);

    while (x && (y = CAR(x))) {
        z = Sv_cons(y, z);
        x = CDR(x);
        if (x && x->type != SV_CONS) {
            z = Sv_cons(x, z);
            break;
        }
    }

    return Sv_reverse(z);
}

extern Sv
*Sv_eval(Env *env, Sv *x)
{
    Sv *y = NULL;

    if (!x)
        return x;

    switch (x->type) {
    case SV_SYM:
        /*
         * If the symbol exists but it's value is NULL, then it is
         * the empty list.
         */
        if ((y = Env_top_get(env, x)) == NULL && !Env_exists(env, x)) {
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
    short varargs = 0;
    Sv *formals, *formal, *arg, *partial, *args = a;
    formals = formal = arg = partial = NULL;

    if (!f)
        return f;

    if (f->type == SV_FUNC)
        return f->val.func(env, a);

    if (f->type == SV_LAMBDA) {
        /* Bind the formals to the arguments. */
        formals = f->val.ufunc->formals;

        while (formals && (formal = CAR(formals))) {
            if (!varargs && formal->type == SV_SYM && *formal->val.buf == '&') {
                varargs = 1;
            } else {
                /* Get the next arg. */
                if (args && (arg = CAR(args))) {
                    if (varargs) {
                        Env_put(f->val.ufunc->env, formal, Sv_list(args));
                        break;
                    } else {
                        Env_put(f->val.ufunc->env, formal, arg);
                    }
                } else if (varargs) {
                    /* Variable args not supplied. */
                    Env_put(f->val.ufunc->env, formal, NULL);
                    break;
                } else {
                    /* No more arguments to consume. */
                    partial = Sv_copy(f);
                    partial->val.ufunc->formals = formals;
                    break;
                }
                args = CDR(args);
            }
            formals = CDR(formals);
        }
        f->val.ufunc->env->parent = env;

        if (partial) {
            return partial;
        } else {
            return Sv_eval(f->val.ufunc->env, f->val.ufunc->body);
        }
    }

    return Sv_new_err("can only call functions");
}
