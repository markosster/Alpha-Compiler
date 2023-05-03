#ifndef LIBFUNCS_H
#define LIBFUNCS_H

#include "memory.h"

#define AVM_LIBFUNCS_HASHSIZE 23

typedef void (*library_func_t)(void);

typedef struct libfuncs_hashtable_bucket
{
    char *id;
    library_func_t library_func;
    struct libfuncs_hashtable_bucket *next;
} libfuncs_hashtable_bucket;

extern libfuncs_hashtable_bucket *libfuncs_hashtable[AVM_LIBFUNCS_HASHSIZE];
extern unsigned libfuncsTotal;

void libfunc_print(void);
void libfunc_input(void);
void libfunc_objectmemberkeys(void);
void libfunc_objecttotalmembers(void);
void libfunc_objectcopy(void);
void libfunc_totalarguments(void);
void libfunc_argument(void);
void libfunc_typeof(void);
void libfunc_strtonum(void);
void libfunc_sqrt(void);
void libfunc_cos(void);
void libfunc_sin(void);

void avm_call_libfunc(char *libfunc_name);

void avm_libfuncs_init();
void avm_libfuncs_register(char *id, library_func_t libfunc);
libfuncs_hashtable_bucket *avm_libfuncs_get(char *id);
void avm_libfuncs_display(FILE *fout);
void avm_libfuncs_free();

#endif