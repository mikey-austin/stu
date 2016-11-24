#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "env.h"

#define INIT_ENV_SIZE 27

extern Env
*Env_new(void)
{
    Env *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Env_new");

    GC_INIT(new, GC_TYPE_ENV);
    new->parent = NULL;
    new->hash = Hash_new(INIT_ENV_SIZE, NULL);

    return new;
}

extern void
Env_destroy(Env **env)
{
    if (env && *env) {
        Env *to_free = *env;
        if (to_free->hash)
            Hash_destroy(&(to_free->hash));
        *env = NULL;
        free(to_free);
    }
}

extern void
Env_put(Env *env, Sv *key, Sv *val)
{
    Hash_put(env->hash, key->val.buf, (void *) val);
}

extern void
Env_top_put(Env *env, Sv *key, Sv *val)
{
    Env *top = env;
    while (top && top->parent)
        top = top->parent;
    Hash_put(top->hash, key->val.buf, (void *) val);
}

extern Sv
*Env_top_get(Env *env, Sv *key)
{
    Sv *val = NULL;

    if ((val = Hash_get(env->hash, key->val.buf)) == NULL
        && env->parent)
    {
        /* Check parent environment. */
        return Env_get(env->parent, key);
    }

    return val;
}

extern int
Env_exists(Env *env, Sv *key)
{
    return Hash_exists(env->hash, key->val.buf);
}

extern Sv
*Env_get(Env *env, Sv *key)
{
    return Hash_get(env->hash, key->val.buf);
}

extern Sv
*Env_del(Env *env, Sv *key)
{
    Sv *val = Env_get(env, key);
    Hash_del(env->hash, key->val.buf);
    return val;
}

extern void
Env_copy(Env *src, Env *dst)
{
    Sv *val = NULL;
    char **keys = NULL;
    int num_keys, i;

    if ((num_keys = Hash_keys(src->hash, &keys)) > 0) {
        for (i = 0; i < num_keys; i++) {
            val = Hash_get(src->hash, keys[i]);
            Hash_put(dst->hash, keys[i], Sv_copy(val));
            free(keys[i]);
        }
        free(keys);
    }
}
