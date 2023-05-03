#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "consts.h"

/* -- AVM instructions -- */
typedef enum vmopcode
{
    assign_v = 0,
    add_v = 1,
    sub_v = 2,
    mul_v = 3,
    div_v = 4,
    mod_v = 5,
    uminus_v = 6,
    and_v = 7,
    or_v = 8,
    not_v = 9,
    jeq_v = 10,
    jne_v = 11,
    jle_v = 12,
    jge_v = 13,
    jlt_v = 14,
    jgt_v = 15,
    call_v = 16,
    pusharg_v = 17,
    funcenter_v = 18,
    funcexit_v = 19,
    newtable_v = 20,
    tablegetelem_v = 21,
    tablesetelem_v = 22,
    jump_v = 23,
    nop_v = 24
} vmopcode;

typedef enum vmarg_t
{
    label_a = 0,
    global_a = 1,
    formal_a = 2,
    local_a = 3,
    number_a = 4,
    string_a = 5,
    bool_a = 6,
    nil_a = 7,
    userfunc_a = 8,
    libfunc_a = 9,
    retval_a = 10,
    none_a = 11
} vmarg_t;

typedef struct vmarg
{
    vmarg_t type;
    unsigned int val;
    char *var_name;
} vmarg;

typedef struct Instruction
{
    vmopcode opcode;
    vmarg *result;
    vmarg *arg1;
    vmarg *arg2;
    unsigned int srcLine;
} Instruction;

/* ------ Instructions ------ */
extern Instruction *instructions;
extern unsigned int totalInst;
extern unsigned int currInst;
#define EXPAND_SIZE_INSTR 1024
#define CURR_SIZE_INSTR (totalInst * sizeof(struct Instruction))
#define NEW_SIZE_INSTR (EXPAND_SIZE_INSTR * sizeof(struct Instruction) + CURR_SIZE_INSTR)
/* -------------------------- */

extern vmopcode vmopcode_map[];
extern const char *vmopcode_map_toString[];
extern const char *vmargType_map_toString[];

void Instr_expand(void);
void Instr_emit(Instruction i);
unsigned int Instr_nextInstr(void);
void Instr_display(FILE *fout, int mode);
void Instr_free(void);

const char *vmopcode_toString(vmopcode opcode);
const char *vmargType_toString(vmarg_t arg_t);

/*typedef struct IncompleteJump
{
    unsigned int instrNo;
    unsigned int iaddress;
    struct IncompleteJump *next;
} IncompleteJump;

extern IncompleteJump *ij_head;
extern IncompleteJump *ij_last;
extern unsigned int ij_total;

IncompleteJump *IncJump_insert(unsigned int instrNo, unsigned int iaddress);
void IncJump_patch(unsigned int instrNo, unsigned int iaddress);
void IncJump_free(void);*/

#endif