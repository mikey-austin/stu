#ifndef SYMTAB_DEFINED
#define SYMTAB_DEFINED

struct Stu;

extern long Symtab_get_id(struct Stu *, const char *);
extern char *Symtab_get_name(struct Stu *, long);
extern void Symtab_init(struct Stu *);
extern void Symtab_destroy(struct Stu *);

#endif
