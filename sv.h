#ifndef SV_DEFINED
#define SV_DEFINED

#define SV_SEXP_REGISTERS 2
#define SV_CAR 0
#define SV_CDR 1

/* Types of stutter values. */
enum Sv_type {
    SV_ERR,
    SV_SYM,
    SV_INT,
    SV_STR,
    SV_SEXP,
    SV_FUNC
};

/* Actual value container. */
struct Sv;
union Sv_val {
    int i;
    char *buf;
    struct Sv *reg[SV_SEXP_REGISTERS];
};

/* Core stutter value. */
typedef struct Sv {
    char gc;
    enum Sv_type type;
    union Sv_val val;
} Sv;

extern Sv *Sv_new(enum Sv_type);
extern Sv *Sv_new_int(int);
extern Sv *Sv_new_str(const char *);
extern Sv *Sv_new_sym(const char *);
extern Sv *Sv_new_err(const char *);

extern void Sv_destroy(Sv **);
extern Sv *Sv_cons(Sv *, Sv *);
extern Sv *Sv_cdr(Sv *);
extern Sv *Sv_car(Sv *);

#endif
