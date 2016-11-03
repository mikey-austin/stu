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
    char *str;
    struct Sv **node;
};

/* Core stutter value. */
typedef struct Sv {
    enum Sv_type type;
    union Sv_val value;
} Sv;

extern Sv *Sv_new(enum Sv_type, union Sv_val *);
extern int Sv_destroy(Sv *);
