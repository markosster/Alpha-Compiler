#ifndef OFFSETSTACK_H
#define OFFSETSTACK_H

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

typedef struct OffsetStack
{
    unsigned int value;
    struct OffsetStack *next;
} OffsetStack;

void OffsetStack_push(OffsetStack **stackTop, unsigned int value);
unsigned int OffsetStack_pop(OffsetStack **stackTop);
unsigned int OffsetStack_top(OffsetStack **stackTop);
int OffsetStack_size(OffsetStack **stackTop);
void OffsetStack_free(OffsetStack **stackTop);
void OffsetStack_display(FILE *fout, OffsetStack **stackTop);

#endif