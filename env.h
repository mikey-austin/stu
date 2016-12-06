#ifndef ENV_DEFINED
#define ENV_DEFINED

#include "hash.h"
#include "sv.h"

#define MAIN_ENV Env_main()

/* Forward declarations. */
struct Gc;
struct Env;

typedef struct Env {
    struct Gc gc;
    struct Env *prev;
    long sym;
    Sv *val;
} Env;

extern Env *Env_new(void);
extern void Env_destroy(Env **);
extern void Env_main_put(Sv *, Sv *);
extern Sv *Env_main_get(Sv *);
extern int Env_main_exists(Sv *);
extern Env *Env_main();
extern Env *Env_put(Env *, Sv *, Sv *);
extern Sv *Env_get(Env *, Sv *);
extern int Env_exists(Env *, Sv *);

#endif
