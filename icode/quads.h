#ifndef QUADS_H
#define QUADS_H

#include "../symtable/symtable.h"
#include "../utils/utils.h"
#include "expr.h"

typedef enum iopcode
{
    jump,
    assign,
    add,
    sub,
    mul,
    div_op,
    mod,
    uminus,
    and,
    or
    ,
    not,
    tablecreate,
    tablegetelem,
    tablesetelem,
    if_eq,
    if_noteq,
    if_greater,
    if_greatereq,
    if_less,
    if_lesseq,
    call,
    param,
    getretval,
    funcstart,
    funcend,
    ret
} iopcode;

typedef struct Quad
{
    enum iopcode op;
    struct Expr *result;
    struct Expr *arg1;
    struct Expr *arg2;
    unsigned int label;
    unsigned int line;
    unsigned int taddress;
} Quad;

/* dynamic array */
extern Quad *quads;
extern unsigned int totalQuads;
extern unsigned int currQuad;
extern unsigned int tempCounter;

#define EXPAND_SIZE 1024
#define CURR_SIZE (totalQuads * sizeof(struct Quad))
#define NEW_SIZE (EXPAND_SIZE * sizeof(struct Quad) + CURR_SIZE)

void Quads_expand(void);
void Quads_emit(iopcode opcode, Expr *result, Expr *arg1, Expr *arg2, unsigned int line);
unsigned int Quads_nextQuad(void);
void Quads_patchLabel(unsigned int quadNo, unsigned int label);
void Quads_display(FILE *out, int color);
void Quads_free(void);

void Quads_okeanos_display(FILE *fout);

/* temp vars */
char *Quads_newTempName(void);
void Quads_resetTemp(void);
SymTableEntry *Quads_newTemp(SymTable *symtable, unsigned int currScope);
unsigned int Quads_isTempVar(Expr *e);
unsigned int Quads_isAnonymFunc(Expr *e);

#endif