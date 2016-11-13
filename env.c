#include <stdlib.h>
#include <err.h>

#include "env.h"

#define INIT_ENV_SIZE 27

extern Env
*Env_new(void)
{
    Env *new = NULL;

    if ((new = malloc(sizeof(*new))) == NULL)
        err(1, "Env_new");

    /* No destructor as the garbage collector will cleanup. */
    new->hash = Hash_new(INIT_ENV_SIZE, NULL);

    return new;
}

extern void
Env_destroy(Env **env)
{
    if (env && *env) {
        if ((*env)->hash)
            Hash_destroy(&((*env)->hash));
        free(*env);
        env = NULL;
    }
}

extern void
Env_put(Env *env, const char *key, Sv *val)
{
    Hash_put(env->hash, key, (void *) val);
}

extern Sv
*Env_get(Env *env, const char *key)
{
    Sv *val = Hash_get(env->hash, key);
    return val;
}

extern Sv
*Env_del(Env *env, const char *key)
{
    Sv *val = Env_get(env, key);
    Hash_del(env->hash, key);
    return val;
}
