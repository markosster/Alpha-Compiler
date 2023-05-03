#ifndef FUNCSTACK_H
#define FUNCSTACK_H

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

#include "symtable.h"

typedef struct FuncStack {
    SymTableEntry *currFuncEntry;
    struct FuncStack *next;
} FuncStack;

void FuncStack_push(FuncStack **stackTop, SymTableEntry *currFuncEntry);
SymTableEntry *FuncStack_pop(FuncStack **stackTop);
SymTableEntry *FuncStack_top(FuncStack **stackTop);
int FuncStack_size(FuncStack **stackTop);
void FuncStack_free(FuncStack **stackTop);
void FuncStack_display(FILE *fout, FuncStack **stackTop);

#endif