#ifndef MEMORY_H
#define MEMORY_H

#include "math.h"
#include "../utils/utils.h"
#include "../tcode/instructions.h"
#include "tables.h"
#include "execute.h"
#include "libfuncs.h"

#define AVM_WARNING(...)                                                 \
    {                                                                    \
        PRINT_PURPLE(stderr, "RUNTIME WARNING at line %u => ", currLine) \
        PRINT_PURPLE(stderr, __VA_ARGS__)                                \
        fprintf(stderr, "\n");                                           \
    }

#define AVM_ERROR(...)                                              \
    {                                                               \
        PRINT_RED(stderr, "RUNTIME ERROR at line %u => ", currLine) \
        PRINT_RED(stderr, __VA_ARGS__)                              \
        fprintf(stderr, "\n");                                      \
        executionFinished = 1;                                      \
    }

#define AVM_TABLE_HASHSIZE 211

typedef enum avm_memcell_t
{
    number_m = 0,
    string_m = 1,
    bool_m = 2,
    table_m = 3,
    userfunc_m = 4,
    libfunc_m = 5,
    nil_m = 6,
    undef_m = 7
} avm_memcell_t;

typedef enum avm_memcell_space
{
    memspace_funcactual,
    memspace_env,
    memspace_funclocal,
    memspace_global,
    memspace_none
} avm_memcell_space;

typedef struct avm_table avm_table;

typedef struct avm_memcell
{
    avm_memcell_t type;
    union
    {
        double numVal;
        char *strVal;
        unsigned char boolVal;
        avm_table *tableVal;
        UserFunc *userfuncVal;
        char *libfuncVal;
    } data;
    avm_memcell_space mem_space;
} avm_memcell;

typedef struct avm_table_bucket
{
    avm_memcell *key;
    avm_memcell *value;
    struct avm_table_bucket *next;
    struct avm_table_bucket *nextInOrder;
} avm_table_bucket;

/*typedef struct avm_table_ordered
{
    avm_table_bucket *ptrBucket;
    struct avm_table_ordered *next;
} avm_table_ordered;*/

struct avm_table
{
    unsigned refCounter;
    avm_table_bucket *tblOrdered;
    avm_table_bucket *strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket *numIndexed[AVM_TABLE_HASHSIZE];
    int hasKeys;
    unsigned total;
};

#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))
#define AVM_STACKENV_SIZE 4
#define AVM_NUMACTUALS_OFFSET 4
#define AVM_SAVEDPC_OFFSET 3
#define AVM_SAVEDTOP_OFFSET 2
#define AVM_SAVEDTOPSP_OFFSET 1

/* ---- Memory Stack ---- */
extern avm_memcell stack[AVM_STACKSIZE];
extern avm_memcell ax, bx, cx;
extern avm_memcell retval;
extern unsigned top, topsp;
extern unsigned totalActuals;
extern unsigned totalGlobals;

/* --- Functions ---- */
int avm_initialize(void);
avm_memcell *avm_translate_operand(vmarg *arg, avm_memcell *reg);
void avm_memcell_assign(avm_memcell *lvalue, avm_memcell *rvalue);

extern char *typeStrings[];

void avm_memcell_clear(avm_memcell *m);
void avm_memclear_string(avm_memcell *m);
void avm_memclear_table(avm_memcell *m);

void avm_dec_top(void);
void avm_push_envvalue(unsigned val);
unsigned avm_get_envvalue(unsigned address);
void avm_callsave_env(void);
unsigned avm_totalactuals(void);
avm_memcell *avm_get_actual(unsigned i);

char *avm_tostring(avm_memcell *m);
unsigned char avm_tobool(avm_memcell *m);

void avm_print_memcell_value(FILE *fout, avm_memcell *m);
void avm_mem_display(FILE *fout);
void avm_mem_free(void);

#endif