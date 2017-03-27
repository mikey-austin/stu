#ifndef SVLIST_DEFINED
#define SVLIST_DEFINED

struct Sv;
struct Env;
struct Svlist_node;
struct Stu;

typedef struct Svlist_node {
    struct Svlist_node *next;
    struct Sv *sv;
} Svlist_node;

typedef struct Svlist {
    int count;
    Svlist_node *head;
    Svlist_node *tail;
} Svlist;

extern Svlist *Svlist_new();
extern void Svlist_destroy(Svlist **);
extern void Svlist_push(Svlist *, struct Sv *);
extern struct Sv *Svlist_eval(struct Stu *, struct Env *, Svlist *);

#endif
