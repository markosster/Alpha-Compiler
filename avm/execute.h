#ifndef EXECUTE_H
#define EXECUTE_H

#include "memory.h"

extern Instruction *code;
extern unsigned char executionFinished;
extern unsigned pc;
extern unsigned currLine;
extern unsigned codeSize;

#define AVM_MAX_INSTRUCTIONS (unsigned)nop_v
#define AVM_ENDING_PC codeSize

extern void execute_ASSIGN(Instruction *i);
extern void execute_ADD(Instruction *i);
extern void execute_SUB(Instruction *i);
extern void execute_MUL(Instruction *i);
extern void execute_DIV(Instruction *i);
extern void execute_MOD(Instruction *i);
extern void execute_UMINUS(Instruction *i);
extern void execute_AND(Instruction *i);
extern void execute_OR(Instruction *i);
extern void execute_NOT(Instruction *i);
extern void execute_JEQ(Instruction *i);
extern void execute_JNE(Instruction *i);
extern void execute_JLE(Instruction *i);
extern void execute_JGE(Instruction *i);
extern void execute_JLT(Instruction *i);
extern void execute_JGT(Instruction *i);
extern void execute_CALL(Instruction *i);
extern void execute_PUSHARG(Instruction *i);
extern void execute_FUNCENTER(Instruction *i);
extern void execute_FUNCEXIT(Instruction *i);
extern void execute_NEWTABLE(Instruction *i);
extern void execute_TABLEGETELEM(Instruction *i);
extern void execute_TABLESETELEM(Instruction *i);
extern void execute_JUMP(Instruction *i);
extern void execute_NOP(Instruction *i);

typedef void (*execute_func_t)(Instruction *);

extern execute_func_t executor[];

void execute_cycle(void);

#endif