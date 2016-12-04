#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "env.h"
#include "symtab.h"

extern Env
*Env_new(void)
{
    Env *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Env_new");

    GC_INIT(new, GC_TYPE_ENV);
    new->parent = NULL;
    new->top = 0;
    new->hash = Hash_new(NULL);

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
    Hash_put(env->hash, Symtab_get_name(key->val.i), (void *) val);
}

extern void
Env_top_put(Env *env, Sv *key, Sv *val)
{
    Env *top = env;
    while (top && top->parent)
        top = top->parent;
    Hash_put(top->hash, Symtab_get_name(key->val.i), (void *) val);
}

extern Sv
*Env_top_get(Env *env, Sv *key)
{
    Hash_ent *ent = NULL;

    if ((ent = Hash_get(env->hash, Symtab_get_name(key->val.i))) == NULL
        && env->parent)
    {
        /* Check parent environment. */
        return Env_top_get(env->parent, key);
    }

    return ent ? ent->v : NULL;
}

extern int
Env_exists(Env *env, Sv *key)
{
    Hash_ent *ent = Hash_get(env->hash, Symtab_get_name(key->val.i));
    return ent != NULL;
}

extern int
Env_top_exists(Env *env, Sv *key)
{
    Hash_ent *ent = NULL;

    if ((ent = Hash_get(env->hash, Symtab_get_name(key->val.i))) == NULL
        && env->parent)
    {
        /* Check parent environment. */
        return Env_top_exists(env->parent, key);
    }

    return ent != NULL;
}

extern Sv
*Env_get(Env *env, Sv *key)
{
    Hash_ent *ent = Hash_get(env->hash, Symtab_get_name(key->val.i));
    return ent ? ent->v : NULL;
}

extern void
Env_del(Env *env, Sv *key)
{
    Hash_del(env->hash, Symtab_get_name(key->val.i));
}

extern void
Env_copy(Env *src, Env *dst)
{
    Hash_ent *cur = Hash_entries(src->hash);
    while (cur) {
        Hash_put(dst->hash, cur->k, Sv_copy((Sv *) cur->v));
        cur = NEXT_ENTRY(cur);
    }
}

extern int
Env_contents(Env *env, Sv ***contents)
{
    Sv **new_contents = NULL;
    int num_entries, i = 0;
    Hash_ent *cur = NULL;

    if ((num_entries = HASH_NUM_ENTRIES(env->hash)) > 0) {
        cur = Hash_entries(env->hash);
        if ((new_contents = calloc(num_entries, sizeof(*new_contents))) == NULL)
            err(1, "Env_entries");
        *contents = new_contents;

        while (cur && i < num_entries) {
            new_contents[i++] = cur->v;
            cur = NEXT_ENTRY(cur);
        }
    }

    return num_entries;
}
