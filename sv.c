#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "symtab.h"
#include "builtins.h"

Sv *Sv_nil = NULL;

extern
void Sv_init(void)
{
    /* Initialize global nil object. */
    Sv_nil = Sv_new(SV_NIL);
    Env_main_put(Sv_new_sym("nil"), Sv_nil);
}

extern Sv
*Sv_new(enum Sv_type type)
{
    Sv *x = NULL;

    if ((x = calloc(1, sizeof(*x))) != NULL) {
        GC_INIT(x, GC_TYPE_SV);
        x->type = type;
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
*Sv_new_float(double f)
{
    Sv *x = Sv_new(SV_FLOAT);
    x->val.f = f;
    return  x;
}

extern Sv
*Sv_new_rational(long n, long d)
{
    if (n % d == 0)
        return Sv_new_int(n / d);

    Sv *x = Sv_new(SV_RATIONAL);
    long max_search = abs(n) > d ? d : abs(n);
    long cur = 2;

    while (cur <= max_search) {
        if ((n % cur == 0) && (d % cur == 0)) {
            n /= cur; d /= cur;
        } else {
            cur++;
        }
    }

    x->val.rational.n = n;
    x->val.rational.d = d;
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
    Sv *x = Sv_new(SV_SYM);
    x->special = !strcmp(sym, "quote")
        || !strcmp(sym, "def")
        || !strcmp(sym, "defmacro")
        || !strcmp(sym, "lambda")
        || !strcmp(sym, "λ")
        || !strcmp(sym, "if");
    x->val.i = Symtab_get_id(sym);
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
*Sv_new_special(enum Sv_special_type type, Sv *body)
{
    Sv *x = Sv_new(SV_SPECIAL);
    Sv_special *s = NULL;

    if ((s = malloc(sizeof(*s))) != NULL) {
        s->type = type;
        s->body = Sv_copy(body);
        x->val.special = s;
    } else {
        err(1, "Sv_new_special");
    }

    return x;
}

extern Sv
*Sv_new_lambda(Env *env, Sv *formals, Sv *body)
{
    Sv *x = Sv_new(SV_LAMBDA);
    Sv_ufunc *f = NULL;

    if ((f = malloc(sizeof(*f))) != NULL) {
        f->env = env;
        f->formals = formals;
        f->body = body;
        f->is_macro = 0;
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
        case SV_STR:
            if ((*sv)->val.buf) {
                free((*sv)->val.buf);
                (*sv)->val.buf = NULL;
            }
            break;

        case SV_CONS:
            /* GC will clean up cons cells. */
            for (int i = 0; i < SV_CONS_REGISTERS; i++)
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
            /* GC will clean up environments and lists. */
            if ((*sv)->val.special) {
                (*sv)->val.special->body = NULL;
                free((*sv)->val.special);
                (*sv)->val.special = NULL;
            }
            break;

        default:
            break;
        }

        free(*sv);
        *sv = NULL;
    }
}

static void
Sv_cons_dump(Sv *sv)
{
    Sv *car = CAR(sv);
    Sv *cdr = CDR(sv);
    Sv_dump(car);
    if (cdr) {
        switch (cdr->type) {
        case SV_CONS:
            printf(" ");
            Sv_cons_dump(cdr);
            break;

        case SV_NIL:
            /* Don't print terminating nil. */
            break;

        default:
            printf(" . ");
            Sv_dump(cdr);
            break;
        }
    } else {
        err(1, "NULL value in CDR");
    }
}

extern void
Sv_dump(Sv *sv)
{
    if (sv) {
        switch (sv->type) {
        case SV_SYM:
            printf("%s", Symtab_get_name(sv->val.i));
            break;

        case SV_ERR:
        case SV_STR:
            if (sv->val.buf)
                printf("\"%s\"", sv->val.buf);
            break;

        case SV_CONS:
            printf("(");
            Sv_cons_dump(sv);
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

            Sv_dump(sv->val.special->body);
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

        case SV_FUNC:
            printf("<builtin>");
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                printf("(λ ");
                Sv_dump(sv->val.ufunc->formals);
                putchar(' ');
                Sv_dump(sv->val.ufunc->body);
                putchar(')');
            }
            break;

        case SV_NIL:
            printf("nil");
            break;
        }
    }
}

static Sv
*Sv_copy_sym(Sv *x)
{
    Sv *y = Sv_new(SV_SYM);
    y->special = x->special;
    y->val.i = x->val.i;
    return y;
}

extern Sv
*Sv_copy(Sv *x)
{
    Sv *y = NULL;

    if (x) {
        switch (x->type) {
        case SV_NIL:
            y = x;
            break;

        case SV_SYM:
            y = Sv_copy_sym(x);
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

        case SV_SPECIAL:
            y = Sv_new_special(x->val.special->type, x->val.special->body);
            break;

        case SV_INT:
            y = Sv_new_int(x->val.i);
            break;

        case SV_FLOAT:
            y = Sv_new_float(x->val.f);
            break;

        case SV_RATIONAL:
            y = Sv_new_rational(x->val.rational.n, x->val.rational.d);
            break;

        case SV_BOOL:
            y = Sv_new_bool((short) x->val.i);
            break;

        case SV_FUNC:
            y = Sv_new_func(x->val.func);
            break;

        case SV_LAMBDA:
            if (x->val.ufunc) {
                y = Sv_new_lambda(
                    x->val.ufunc->env, x->val.ufunc->formals, x->val.ufunc->body);
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
    Sv *y = NIL;

    while (!IS_NIL(x) && x->type == SV_CONS) {
        y = Sv_cons(CAR(x), y);
        x = CDR(x);
    }

    return y;
}

extern Sv
*Sv_list(Sv *x)
{
    Sv *y = NULL, *z = NIL;

    if (!x)
        return x;

    if (x->type != SV_CONS)
        return Sv_cons(x, NIL);

    while (!IS_NIL(x) && (y = CAR(x))) {
        z = Sv_cons(y, z);
        x = CDR(x);
    }

    return Sv_reverse(z);
}

extern Sv
*Sv_expand_1(Sv *x)
{
    if (!x || x->type != SV_CONS)
        return x;

    Sv *head = CAR(x);
    Sv *macro;

    if (head->type != SV_SYM) return x;

    macro = Env_main_get(head);

    if (!macro || macro->type != SV_LAMBDA ||
          !macro->val.ufunc->is_macro) return x;

    return Sv_call(Env_main(), macro, CDR(x));
}

extern Sv
*Sv_expand(Sv *x)
{
    if (!x || x->type != SV_CONS)
        return x;

    Sv *head = CAR(x);
    Sv *macro;

    while (1) {
        if (head->type != SV_SYM) return x;

        macro = Env_main_get(head);

        if (!macro || macro->type != SV_LAMBDA || !macro->val.ufunc->is_macro) {
            return x;
        }

        x = Sv_expand_1(x);
        head = CAR(x);
    }

    // we won't get there
}

extern Sv
*Sv_eval(Env *env, Sv *x)
{
    Sv *y = NULL;

    if (!x)
        return x;

    PUSH_SCOPE;
    switch (x->type) {
    case SV_SYM:
        /*
         * If the symbol exists but it's value is NULL, then it is
         * the empty list.
         */
        if ((y = Env_get(env, x)) == NULL && !Env_exists(env, x)) {
            y = Sv_new_err("possibly unknown symbol");
        }
        break;

    case SV_SPECIAL:
        y = Sv_eval_special(env, x);
        break;

    case SV_CONS:
        y = Sv_eval_sexp(env, x);
        break;

    default:
        y = x;
        break;
    }
    POP_N_SAVE(y);

    return y;
}

extern Sv
*Sv_eval_list(Env *env, Sv *x)
{
    if (IS_NIL(x)) return x;

    Sv *head = Sv_eval(env, CAR(x));

    if (IS_NIL(CDR(x))) {
        return head;
    } else {
        return Sv_eval_list(env, CDR(x));
    }
}

extern Sv
*Sv_eval_special(Env *env, Sv *x)
{
    Sv_special *special = x->val.special;
    Sv *body = x->val.special->body;

    switch (special->type) {
        case SV_SPECIAL_BACKQUOTE:
            if (body->type == SV_SYM) {
                return Sv_eval_sexp(env, Sv_cons(Sv_new_sym("quote"), Sv_cons(body, NIL)));
            } else if (body->type == SV_CONS) {
                return Sv_eval_special_cons(env, body);
            } else {
                return body;
            }
            break;
        case SV_SPECIAL_COMMA:
            return Sv_new_err("Comma can be used only inside backquoted list");
        case SV_SPECIAL_COMMA_SPREAD:
            return Sv_new_err("Comma spread can be used only inside backquoted list");
    }

    return Sv_new_err("Troubles with quoting magic");
}

extern Sv
*Sv_eval_special_cons(Env *env, Sv *x)
{
    if (IS_NIL(x)) return x;

    Sv *head = CAR(x);

    if (head->type == SV_SPECIAL) {
        Sv_special *special = head->val.special;

        switch (special->type) {
            case SV_SPECIAL_COMMA:
                return Sv_cons(Sv_eval(env, special->body),
                        Sv_eval_special_cons(env, CDR(x)));
                break;

            case SV_SPECIAL_COMMA_SPREAD:
                if (special->body->type == SV_SYM || special->body->type == SV_CONS) {
                    Sv *val = Sv_eval(env, special->body);
                    if (val->type == SV_CONS) {
                        return val;
                    } else {
                        return Sv_new_err("spread operator applied to atom");
                    }
                } else {
                    return Sv_new_err("spread operator can be applied only to symbol");
                }

                break;
            case SV_SPECIAL_BACKQUOTE:
                return Sv_new_err("backquote is not permitted inside of other backquote");
        }
    } else if (head->type == SV_CONS)  {
        return Sv_cons(Sv_eval_special_cons(env, head),
                        Sv_eval_special_cons(env, CDR(x)));
    } else {
        return Sv_cons(head, Sv_eval_special_cons(env, CDR(x)));
    }

    return Sv_new_err("Got confused inside of backquoted list");
}

extern Sv
*Sv_eval_sexp(Env *env, Sv *x)
{
    Sv *cur = NULL, *y = NULL, *z = NULL;
    cur = x = Sv_expand(x);
    /*cur = x;*/
    /*Sv_dump(cur);*/

    if (!cur)
        return NULL;

    z = CAR(cur);

    if (z && z->special) {
        /* Only evaluate the head, leaving tail intact. */
        y = Sv_eval(env, z);
    } else {
        /* Evaluate all arguments. */
         while (!IS_NIL(cur)) {
            if (cur->type == SV_CONS) {
                y = Sv_cons(Sv_eval(env, CAR(cur)), y);
                cur = CDR(cur);
            } else {
                y = Sv_cons(cur, y);
                break;
            }
        }
        x = Sv_reverse(y);
        y = CAR(x);
    }

    /* The car should now be a function. */
    if (y && (y->type == SV_FUNC || y->type == SV_LAMBDA)) {
        return Sv_call(env, y, CDR(x));
    }

    return Sv_new_err("first element is not a function");
}

extern Sv
*Sv_call(struct Env *env, Sv *f, Sv *a)
{
    short varargs = 0;
    Env *call_env;
    Sv *formals, *formal, *arg, *partial, *args = a;
    formals = formal = arg = partial = NULL;

    if (!f)
        return f;

    if (f->type == SV_FUNC)
        return f->val.func(env, a);

    if (f->type == SV_LAMBDA) {
        /* Bind the formals to the arguments. */
        formals = f->val.ufunc->formals;
        call_env = f->val.ufunc->env;

        while (!IS_NIL(formals) && (formal = CAR(formals))) {
            if (!varargs && formal->type == SV_SYM && formal->val.i == Symtab_get_id("&")) {
                varargs = 1;
            } else {
                /* Get the next arg. */
                if (args && (arg = CAR(args))) {
                    if (varargs) {
                        call_env = Env_put(call_env, formal, Sv_list(args));
                        break;
                    } else {
                        call_env = Env_put(call_env, formal, arg);
                    }
                } else if (varargs) {
                    /* Variable args not supplied. */
                    call_env = Env_put(call_env, formal, NULL);
                    break;
                } else {
                    /* No more arguments to consume. */
                    partial = Sv_copy(f);
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
            return Sv_eval_list(call_env, f->val.ufunc->body);
        }
    }

    return Sv_new_err("can only call functions");
}
