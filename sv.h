#ifndef SV_DEFINED
#define SV_DEFINED

#define SV_CONS_REGISTERS 2
#define SV_CAR_REG 0
#define SV_CDR_REG 1

#define CAR(sv) ((sv) ? (sv)->val.reg[SV_CAR_REG] : NULL)
#define CDR(sv) ((sv) ? (sv)->val.reg[SV_CDR_REG] : NULL)

/* Types of stutter values. */
enum Sv_type {
    SV_ERR,
    SV_SYM,
    SV_INT,
    SV_BOOL,
    SV_STR,
    SV_CONS,
    SV_FUNC,
    SV_LAMBDA
};

/* Forward declarations. */
struct Env;
struct Sv;

/* Function value definition. */
typedef struct Sv *(*Sv_func)(struct Env *, struct Sv *);

typedef struct Sv_ufunc {
    struct Env *env;
    struct Sv *formals;
    struct Sv *body;

} Sv_ufunc;

/* Actual value container. */
union Sv_val {
    long i;
    char *buf;
    Sv_func func;
    struct Sv *reg[SV_CONS_REGISTERS];
    struct Sv_ufunc *ufunc;
};

/* Core stutter value. */
typedef struct Sv {
    short special;
    enum Sv_type type;
    union Sv_val val;
} Sv;

extern Sv *Sv_new(enum Sv_type);
extern Sv *Sv_new_int(long);
extern Sv *Sv_new_bool(short);
extern Sv *Sv_new_str(const char *);
extern Sv *Sv_new_sym(const char *);
extern Sv *Sv_new_err(const char *);
extern Sv *Sv_new_func(Sv_func);
extern Sv *Sv_new_lambda(Sv *, Sv *);

extern void Sv_dump(Sv *sv);
extern void Sv_destroy(Sv **);
extern Sv *Sv_cons(Sv *, Sv *);
extern Sv *Sv_reverse(Sv *);

extern Sv *Sv_eval(struct Env *, Sv *);
extern Sv *Sv_eval_sexp(struct Env *, Sv *);
extern Sv *Sv_call(struct Env *, Sv *, Sv *);

#endif
