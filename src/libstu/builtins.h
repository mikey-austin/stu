#ifndef BUILTINS_DEFINED
#define BUILTINS_DEFINED

#include "gc.h"
#include "env.h"
#include "sv.h"

struct Stu;

extern void Builtin_init(struct Stu *);
extern Sv *Builtin_add(struct Stu *, Env *, Sv *);
extern Sv *Builtin_sub(struct Stu *, Env *, Sv *);
extern Sv *Builtin_mul(struct Stu *, Env *, Sv *);
extern Sv *Builtin_div(struct Stu *, Env *, Sv *);
extern Sv *Builtin_quote(struct Stu *, Env *, Sv *);
extern Sv *Builtin_def(struct Stu *, Env *, Sv *);
extern Sv *Builtin_cons(struct Stu *, Env *, Sv *);
extern Sv *Builtin_list(struct Stu *, Env *, Sv *);
extern Sv *Builtin_lambda(struct Stu *, Env *, Sv *);
extern Sv *Builtin_defmacro(struct Stu *, Env *, Sv *);
extern Sv *Builtin_macroexpand_1(struct Stu *, Env *, Sv *);
extern Sv *Builtin_macroexpand(struct Stu *, Env *, Sv *);
extern Sv *Builtin_progn(struct Stu *, Env *, Sv *);
extern Sv *Builtin_eval(struct Stu *, Env *, Sv *);
extern Sv *Builtin_car(struct Stu *, Env *, Sv *);
extern Sv *Builtin_cdr(struct Stu *, Env *, Sv *);
extern Sv *Builtin_reverse(struct Stu *, Env *, Sv *);
extern Sv *Builtin_if(struct Stu *, Env *, Sv*);
extern Sv *Builtin_eq(struct Stu *, Env *, Sv*);
extern Sv *Builtin_gt(struct Stu *, Env *, Sv*);
extern Sv *Builtin_lt(struct Stu *, Env *, Sv*);
extern Sv *Builtin_gte(struct Stu *, Env *, Sv*);
extern Sv *Builtin_lte(struct Stu *, Env *, Sv*);
extern Sv *Builtin_read(struct Stu *, Env *, Sv*);
extern Sv *Builtin_print(struct Stu *, Env *, Sv*);

#endif
