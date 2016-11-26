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
    Hash_ent **entries = NULL;
    int num_entries, i;

    if ((num_entries = Hash_entries(src->hash, &entries)) > 0) {
        for (i = 0; i < num_entries; i++)
            Hash_put(dst->hash, entries[i]->k, Sv_copy((Sv *) entries[i]->v));
        free(entries);
    }
}

extern int
Env_contents(Env *env, Sv ***contents)
{
    Hash_ent **entries = NULL;
    Sv **new_contents = NULL;
    int num_entries, i;

    if ((num_entries = Hash_entries(env->hash, &entries)) > 0) {
        if ((new_contents = calloc(num_entries, sizeof(*new_contents))) == NULL)
            err(1, "Env_entries");
        *contents = new_contents;

        for (i = 0; i < num_entries; i++)
            new_contents[i] = entries[i]->v;
        free(entries);
    }

    return num_entries;
}
