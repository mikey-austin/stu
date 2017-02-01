#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "env.h"
#include "symtab.h"

static Env *main_env = NULL;

extern Env
*Env_new(void)
{
    Env *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Env_new");

    GC_INIT(new, GC_TYPE_ENV);

    return new;
}

extern void
Env_destroy(Env **env)
{
    Env *e = *env;
    if (env && e) {
        free(*env);
        *env = NULL;
    }
}

extern Env
*Env_put(Env *env, Sv *key, Sv *val)
{
    if (key) {
        Env *new = Env_new();
        new->sym = key->val.i;
        new->prev = env;
        new->val = val;
        return new;
    }

    return env;
}

extern Env
*Env_main_put(Sv *key, Sv *val)
{
    main_env = Env_put(main_env, key, val);
    return main_env;
}

extern Sv
*Env_main_get(Sv *key)
{
    return main_env ? Env_get(main_env, key) : NULL;
}

extern Env
*Env_main()
{
    return main_env;
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
Env_main_exists(Sv *key)
{
    return Env_exists(main_env, key);
}

extern Sv
*Env_get(Env *env, Sv *key)
{
    Env *cur = env;

    for (; key && cur; cur = cur->prev) {
        if (cur->sym == key->val.i)
            return cur->val;
    }

    return NULL;
}
