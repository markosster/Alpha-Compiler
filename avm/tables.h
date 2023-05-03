#ifndef TABLES_H
#define TABLES_H

#include "memory.h"

typedef struct avm_table avm_table;
typedef struct avm_table_bucket avm_table_bucket;
typedef struct avm_memcell avm_memcell;

extern int tableSetElemNil;

avm_table *avm_table_new(void);
void avm_table_destroy(avm_table *tab);
void avm_table_inc_refcounter(avm_table *table);
void avm_table_dec_refcounter(avm_table *table);
void avm_table_buckets_init(avm_table_bucket **p);
void avm_table_buckets_destroy(avm_table_bucket **p);

void avm_table_delelem(avm_table *table, avm_memcell *elem);
avm_memcell *avm_table_getelem(avm_table *table, avm_memcell *index);
void avm_table_setelem(avm_table *table, avm_memcell *index, avm_memcell *rvalue);

avm_memcell *avm_table_functor(avm_memcell *func);

void avm_table_display(avm_table *table);

#endif
