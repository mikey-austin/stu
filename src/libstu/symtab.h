#ifndef SYMTAB_DEFINED
#define SYMTAB_DEFINED

extern long Symtab_get_id(const char *);
extern char *Symtab_get_name(long);
extern void Symtab_init(void);
extern void Symtab_destroy(void);

#endif
