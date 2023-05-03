#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "scopespaces.h"
#include "../utils/utils.h"
#include "../icode/quadlists.h"

#define HASH_MULTIPLIER 65599
#define BUCKETS 509

#define true 1
#define false 0

typedef enum SymbolType
{
    GLOBAL,
    LOCAL,
    FORMAL,
    USERFUNC,
    LIBFUNC
} SymbolType;

typedef struct Variable
{
    void *val;
} Variable;

typedef struct Function
{
    int total_locals;
    int total_args;
    unsigned int icode_addr;
    unsigned int tcode_addr;
    struct QuadList *returnList;
} Function;

typedef struct SymTableEntry
{
    const char *name;
    unsigned int scope;
    unsigned int line;
    int isActive;
    enum SymbolType type;
    enum ScopeSpaceType scopeSpace;
    int offset;

    struct
    {
        struct Variable *varVal;
        struct Function *funcVal;
    } value;

    struct SymTableEntry *nextInCoList;
    struct SymTableEntry *nextInScopeList;
} SymTableEntry;

typedef struct ScopeListEntry
{
    unsigned int scopeNum;
    struct SymTableEntry *headScopeSymEntry;
    struct SymTableEntry *lastScopeSymEntry;
    struct ScopeListEntry *next;
} ScopeListEntry;

typedef struct ScopeList
{
    struct ScopeListEntry *headScopeListEntry;
    struct ScopeListEntry *lastScopeListEntry;
} ScopeList;

typedef struct SymTable
{
    struct SymTableEntry *table[BUCKETS];
    unsigned int size;
    struct ScopeList *scopeList;
} SymTable;

/* -------------------- SYMBOL TABLE -------------------- */

SymTable *SymTable_init(void);

void SymTable_free(SymTable *symtab);

SymTableEntry *SymTable_insert(SymTable *symtab, const char *name, unsigned int scope, unsigned int line, int isActive, Variable *varVal, Function *funcVal, SymbolType type);

void SymTable_display(FILE *fout, SymTable *symtab);

/* -------------------- SCOPE LIST -------------------- */

int ScopeList_insert(ScopeList *scopelist, SymTableEntry *symtabEntry, unsigned int scope);

SymTableEntry *ScopeList_lookup(ScopeList *scopelist, const char *name, unsigned int scope);

SymTableEntry *ScopeList_lastlookup(ScopeList *scopelist, const char *name, unsigned int scope);

ScopeListEntry *ScopeList_get(ScopeList *scopelist, unsigned int scope);

int ScopeList_hide(ScopeList *scopelist, unsigned int scope);

void ScopeList_display(FILE *fout, SymTable *symtab);

/* -------------------- TYPES -------------------- */

Function *Function_new(void);

Variable *Variable_new(void);

#endif