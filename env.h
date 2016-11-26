#ifndef ENV_DEFINED
#define ENV_DEFINED

#include "hash.h"
#include "sv.h"

/* Forward declarations. */
struct Gc;
struct Env;

typedef struct Env {
    struct Gc gc;
    struct Env *parent;
    Hash *hash;
} Env;

extern Env *Env_new(void);
extern void Env_destroy(Env **);
extern void Env_top_put(Env *, Sv *, Sv *);
extern void Env_put(Env *, Sv *, Sv *);
extern Sv *Env_top_get(Env *, Sv *);
extern Sv *Env_get(Env *, Sv *);
extern int Env_exists(Env *, Sv *);
extern Sv *Env_del(Env *, Sv *);
extern void Env_copy(Env *, Env *);
extern int Env_contents(Env *, Sv ***);

#endif
