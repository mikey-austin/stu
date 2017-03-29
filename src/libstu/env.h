#ifndef ENV_DEFINED
#define ENV_DEFINED

#include "hash.h"
#include "sv.h"

/* Forward declarations. */
struct Gc;
struct Env;
struct Stu;

typedef struct Env {
    struct Gc gc;
    struct Env *prev;
    long sym;
    Sv *val;
} Env;

extern Env *Env_new(struct Stu *);
extern void Env_destroy(Env **);
extern Env *Env_main_put(struct Stu *, Sv *, Sv *);
extern Sv *Env_main_get(struct Stu *, Sv *);
extern int Env_main_exists(struct Stu *, Sv *);
extern Env *Env_main(struct Stu *);
extern Env *Env_put(struct Stu *, Env *, Sv *, Sv *);
extern Sv *Env_get(Env *, Sv *);
extern int Env_exists(Env *, Sv *);

#endif
