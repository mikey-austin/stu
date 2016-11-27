#ifndef BUILTINS_DEFINED
#define BUILTINS_DEFINED

#include "env.h"
#include "sv.h"

extern void Builtin_install(Env *);

extern Sv *Builtin_add(Env *, Sv *);
extern Sv *Builtin_sub(Env *, Sv *);
extern Sv *Builtin_mul(Env *, Sv *);
extern Sv *Builtin_div(Env *, Sv *);
extern Sv *Builtin_quote(Env *, Sv *);
extern Sv *Builtin_def(Env *, Sv *);
extern Sv *Builtin_cons(Env *, Sv *);
extern Sv *Builtin_list(Env *, Sv *);
extern Sv *Builtin_lambda(Env *, Sv *);
extern Sv *Builtin_eval(Env *, Sv *);
extern Sv *Builtin_car(Env *, Sv *);
extern Sv *Builtin_cdr(Env *, Sv *);
extern Sv *Builtin_reverse(Env *, Sv *);
extern Sv *Builtin_if(Env *, Sv*);
extern Sv *Builtin_not(Env *, Sv*);
extern Sv *Builtin_eq(Env *, Sv*);
extern Sv *Builtin_gt(Env *, Sv*);
extern Sv *Builtin_lt(Env *, Sv*);
extern Sv *Builtin_gte(Env *, Sv*);
extern Sv *Builtin_lte(Env *, Sv*);
extern Sv *Builtin_nilp(Env *, Sv*);

#endif
