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

#include "alloc/alloc.h"
#include "stu_private.h"
#include "gc.h"
#include "env.h"
#include "symtab.h"

extern Env
*Env_new(Stu *stu)
{
    Env *new = NULL;

    if ((new = Alloc_allocate(stu->env_alloc)) == NULL)
        err(1, "Env_new");
    GC_INIT(stu, new, GC_TYPE_ENV);

    return new;
}

extern void
Env_destroy(Stu *stu, Env **env)
{
    Env *e = *env;
    if (env && e) {
        Alloc_release(stu->env_alloc, *env);
        *env = NULL;
    }
}

extern Env
*Env_put(Stu *stu, Env *env, Sv *key, Sv *val)
{
    if (key) {
        Env *new = Env_new(stu);
        new->sym = key->val.i;
        new->prev = env;
        new->val = val;
        return new;
    }

    return env;
}

extern Env
*Env_main_put(Stu *stu, Sv *key, Sv *val)
{
    stu->main_env = Env_put(stu, stu->main_env, key, val);
    return stu->main_env;
}

extern Sv
*Env_main_get(Stu *stu, Sv *key)
{
    return stu->main_env ? Env_get(stu->main_env, key) : NULL;
}

extern Env
*Env_main(Stu *stu)
{
    return stu->main_env;
}

extern int
Env_exists(Env *env, Sv *key)
{
    Env *cur;

    if (key) {
        for (cur = env; cur; cur = cur->prev) {
            if (cur->sym == key->val.i)
                return 1;
        }
    }

    return 0;
}

extern int
Env_main_exists(Stu *stu, Sv *key)
{
    return Env_exists(stu->main_env, key);
}

extern Sv
*Env_get(Env *env, Sv *key)
{
    Env *cur = env;

    for (; key && key->type == SV_SYM && cur; cur = cur->prev) {
        if (cur->sym == key->val.i)
            return cur->val;
    }

    return NULL;
}
