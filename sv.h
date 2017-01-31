#ifndef SV_DEFINED
#define SV_DEFINED

#define SV_CONS_REGISTERS 2
#define SV_CAR_REG 0
#define SV_CDR_REG 1
#define NIL        Sv_nil

#define IS_NIL(sv) ((sv) && (sv)->type == SV_NIL ? 1 : ((sv) ? 0 : 1))
#define CAR(sv)    ((sv) && (sv)->type == SV_CONS ? (sv)->val.reg[SV_CAR_REG] : NULL)
#define CDR(sv)    ((sv) && (sv)->type == SV_CONS ? (sv)->val.reg[SV_CDR_REG] : NULL)
#define CADR(sv)   ((sv) ? CAR(CDR((sv))) : NULL)
#define CADDR(sv)  ((sv) ? CAR(CDR(CDR((sv)))) : NULL)

/* Types of stutter values. */
enum Sv_type {
    SV_NIL,
    SV_ERR,
    SV_SYM,
    SV_INT,
    SV_FLOAT,
    SV_RATIONAL,
    SV_BOOL,
    SV_STR,
    SV_CONS,
    SV_FUNC,
    SV_LAMBDA
};

/* Forward declarations. */
struct Gc;
struct Env;
struct Sv;

/* Function value definition. */
typedef struct Sv *(*Sv_func)(struct Env *, struct Sv *);

typedef struct Sv_ufunc {
    struct Env *env;
    struct Sv *formals;
    struct Sv *body;
    short  is_macro;
} Sv_ufunc;

typedef struct Sv_rational {
    long n;
    long d;
} Sv_rational;

/* Actual value container. */
union Sv_val {
    long i;
    double f;
    char *buf;
    Sv_func func;
    Sv_rational rational;
    struct Sv *reg[SV_CONS_REGISTERS];
    struct Sv_ufunc *ufunc;
};

/* Core stutter value. */
typedef struct Sv {
    struct Gc gc;
    unsigned char special;
    enum Sv_type type;
    union Sv_val val;
} Sv;

/* Global nil object. */
extern Sv *Sv_nil;

extern void Sv_init(void);
extern Sv *Sv_new(enum Sv_type);
extern Sv *Sv_new_int(long);
extern Sv *Sv_new_float(double);
extern Sv *Sv_new_rational(long, long);
extern Sv *Sv_new_bool(short);
extern Sv *Sv_new_str(const char *);
extern Sv *Sv_new_sym(const char *);
extern Sv *Sv_new_err(const char *);
extern Sv *Sv_new_func(Sv_func);
extern Sv *Sv_new_lambda(struct Env *, Sv *, Sv *);

extern void Sv_dump(Sv *sv);
extern void Sv_destroy(Sv **);
extern Sv *Sv_cons(Sv *, Sv *);
extern Sv *Sv_reverse(Sv *);
extern Sv *Sv_list(Sv *);
extern Sv *Sv_copy(Sv *);

extern Sv *Sv_eval(struct Env *, Sv *);
extern Sv *Sv_eval_sexp(struct Env *, Sv *);
extern Sv *Sv_call(struct Env *, Sv *, Sv *);

#endif
