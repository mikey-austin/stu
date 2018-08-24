/*
 * Copyright (c) 2018 Mikey Austin <mikey@jackiemclean.net>
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
#include <stdio.h>
#include <setjmp.h>

#include "env.h"
#include "stu_private.h"
#include "try.h"
#include "sv.h"
#include "gc.h"

static Sv
*try(Stu *stu, Env *env, Sv *to_eval, Sv *(*catch)(Stu *stu, Sv *e, Env *env, void *arg),
     void *catch_arg, Sv *(*eval)(Stu *stu, Env *env, Sv *x))
{
    Sv *result = NIL;
    int jmp_res = 0;
    jmp_buf curr_marker;

    volatile int try_scope_stack_pos = Gc_scope_stack_size(stu);
    jmp_buf *prev_marker = stu->last_try_marker;
    stu->last_try_marker = &curr_marker;

    if ((jmp_res = setjmp(curr_marker)) == 0) {
        result = eval(stu, env, to_eval);
    } else {
        /*
         * We have "caught" an exception, so invoke the supplied catch
         * routine with the exception as an argument.
         */
        PUSH_N_SAVE(stu, stu->last_exception);
        result = catch(stu, stu->last_exception, env, catch_arg);
        POP_SCOPE(stu);

        /*
         * Cleanup the no-longer-needed scope stacks between the try
         * and the throw, and let the garbage collector re-claim the
         * objects.
         */
        int curr_scope_stack_pos = Gc_scope_stack_size(stu);
        int to_unwind = curr_scope_stack_pos - try_scope_stack_pos;
        for (int i = to_unwind; i > 0; i--) {
            POP_SCOPE(stu);
        }

        /*
         * Sv_eval saves the result in the parent scope for us, however
         * in this special case we had to manually pop potentially more
         * than one scope due to the longjmp. Explicitly saving in the
         * current scope here achieves the same effect as a regular call
         * to Sv_eval.
         */
        SCOPE_SAVE(stu, result);
    }

    /* Re-instate the previous try marker. */
    stu->last_try_marker = prev_marker;
    stu->last_exception = NIL;

    return result;
}

/*
 * This catch handler takes a lambda as an argument and applies it with
 * the supplied exception as the first argument.
 */
static Sv
*apply_lambda_catch_handler(Stu *stu, Sv *exception, Env *env, void *arg)
{
    Sv *catch_lambda = arg;
    Sv *handler = Sv_cons(
        stu, catch_lambda, Sv_cons(
            stu, Sv_cons(
                stu, Sv_new_sym(stu, "quote"), Sv_cons(
                    stu, exception, NIL)), NIL));
    return Sv_eval(stu, env, handler);
}

extern Sv
*Try_eval_stu_catch(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'try' args is not a cons");

    Sv *to_eval = CAR(args);
    Sv *catch_lambda = CADR(args);

    return Try_eval(
        stu, env, to_eval, apply_lambda_catch_handler, (void *) catch_lambda);
}

extern Sv
*Try_eval(Stu *stu, Env *env, Sv *to_eval, Sv *(*catch)(Stu *, Sv *, Env *, void *), void *catch_arg)
{
    return try(stu, env, to_eval, catch, catch_arg, Sv_eval);
}

static inline Sv
*eval_list_ignore_updated_env(Stu *stu, Env *env, Sv *x)
{
    return Sv_eval_list(stu, env, x, NULL);
}

extern Sv
*Try_eval_list(Stu *stu, Env *env, Sv *to_eval, Sv *(*catch)(Stu *, Sv *, Env *, void *), void *catch_arg)
{
    return try(stu, env, to_eval, catch, catch_arg, eval_list_ignore_updated_env);
}

extern Sv
*Try_default_catch_handler(Stu *stu, Sv *e, Env *env, void *arg)
{
    fprintf(stderr, "An uncaught exception occured:\n\n");
    Sv_dump(stu, CAR(e), stderr);
    fprintf(stderr, "\n\nStack trace:\n");

    Sv *frame = CDR(e);
    for (int i = 0; !IS_NIL(frame); i++, frame = CDR(frame)) {
        Sv *fun_name = CAR(frame);
        if (!IS_NIL(fun_name)) {
            fprintf(stderr, "%4d: %s\n", i, fun_name->val.buf);
        }
    }

    // Terminate program with non-success.
    exit(1);

    // Not reached.
    return NIL;
}
