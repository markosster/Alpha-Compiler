#ifndef GENERATE_H
#define GENERATE_H

#include "instructions.h"
#include "../icode/quads.h"

extern void generate_ADD(Quad *);
extern void generate_SUB(Quad *);
extern void generate_MUL(Quad *);
extern void generate_DIV(Quad *);
extern void generate_MOD(Quad *);
extern void generate_UMINUS(Quad *);
extern void generate_NEWTABLE(Quad *);
extern void generate_TABLEGETELEM(Quad *);
extern void generate_TABLESETELEM(Quad *);
extern void generate_ASSIGN(Quad *);
extern void generate_NOP(Quad *);
extern void generate_JUMP(Quad *);
extern void generate_IF_EQ(Quad *);
extern void generate_IF_NOTEQ(Quad *);
extern void generate_IF_GREATER(Quad *);
extern void generate_IF_GREATEREQ(Quad *);
extern void generate_IF_LESS(Quad *);
extern void generate_IF_LESSEQ(Quad *);
extern void generate_NOT(Quad *);
extern void generate_OR(Quad *);
extern void generate_AND(Quad *);
extern void generate_PARAM(Quad *);
extern void generate_CALL(Quad *);
extern void generate_GETRETVAL(Quad *);
extern void generate_FUNCSTART(Quad *);
extern void generate_FUNCEND(Quad *);
extern void generate_RETURN(Quad *);

typedef void (*generator_t)(Quad *);

extern generator_t generators[];

void Tcode_makeOperand(Expr *expr, vmarg *arg);
void Tcode_makeRetValOperand(vmarg *arg);

void generate_tcode (void);

#endif
