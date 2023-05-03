#include "instructions.h"

Instruction *instructions = (Instruction *)0;
unsigned int totalInst = 0;
unsigned int currInst = 0;
/*IncompleteJump *ij_head = (IncompleteJump *)0;
IncompleteJump *ij_last = (IncompleteJump *)0;
unsigned int ij_total = 0;*/

vmopcode vmopcode_map[] = {
    assign_v,
    add_v,
    sub_v,
    mul_v,
    div_v,
    mod_v,
    uminus_v,
    and_v,
    or_v,
    not_v,
    jeq_v,
    jne_v,
    jle_v,
    jge_v,
    jlt_v,
    jgt_v,
    call_v,
    pusharg_v,
    funcenter_v,
    funcexit_v,
    newtable_v,
    tablegetelem_v,
    tablesetelem_v,
    jump_v,
    nop_v};

const char *vmopcode_map_toString[] = {
    "ASSIGN",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "UMINUS",
    "AND",
    "OR",
    "NOT",
    "JEQ",
    "JNE",
    "JLE",
    "JGE",
    "JLT",
    "JGT",
    "CALL",
    "PUSHARG",
    "FUNCENTER",
    "FUNCEXIT",
    "NEWTABLE",
    "TABLEGETELEM",
    "TABLESETELEM",
    "JUMP",
    "NOP"};

const char *vmargType_map_toString[] = {
    "label",
    "global",
    "formal",
    "local",
    "number",
    "string",
    "bool",
    "nil",
    "userfunc",
    "libfunc",
    "retval",
    ""};

const char *vmopcode_toString(vmopcode opcode)
{
    return vmopcode_map_toString[opcode];
    /*switch (opcode)
    {
    case assign_v:
        return "ASSIGN";
    case add_v:
        return "ADD";
    case sub_v:
        return "SUB";
    case mul_v:
        return "MUL";
    case div_v:
        return "DIV";
    case mod_v:
        return "MOD";
    case uminus_v:
        return "UMINUS";
    case and_v:
        return "AND";
    case or_v:
        return "OR";
    case not_v:
        return "NOT";
    case jeq_v:
        return "JEQ";
    case jne_v:
        return "JNE";
    case jle_v:
        return "JLE";
    case jge_v:
        return "JGE";
    case jlt_v:
        return "JLT";
    case jgt_v:
        return "JGT";
    case call_v:
        return "CALL";
    case pusharg_v:
        return "PUSHARG";
    case funcenter_v:
        return "FUNCENTER";
    case funcexit_v:
        return "FUNCEXIT";
    case newtable_v:
        return "NEWTABLE";
    case tablegetelem_v:
        return "TABLEGETELEM";
    case tablesetelem_v:
        return "TABLESETELEM";
    case jump_v:
        return "JUMP";
    default:
        return "";
    }*/
}

const char *vmargType_toString(vmarg_t arg_t)
{
    return vmargType_map_toString[arg_t];
    /*switch (arg_t)
    {
    case label_a:
        return "label";
    case global_a:
        return "global";
    case formal_a:
        return "formal";
    case local_a:
        return "local";
    case number_a:
        return "number";
    case string_a:
        return "string";
    case bool_a:
        return "bool";
    case nil_a:
        return "nil";
    case userfunc_a:
        return "userfunc";
    case libfunc_a:
        return "libfunc";
    case retval_a:
        return "retval";
    default:
        return "";
    }*/
}

static void print_arg_value(FILE *fout, vmarg *arg)
{
    switch (arg->type)
    {
    case number_a:
        fprintf(fout, "%d(%s):%u(%g) ", arg->type, vmargType_toString(arg->type), arg->val, NumConsts_get(arg->val));
        break;
    case string_a:
        fprintf(fout, "%d(%s):%u(%s) ", arg->type, vmargType_toString(arg->type), arg->val, StrConsts_get(arg->val));
        break;
    case userfunc_a:
        fprintf(fout, "%d(%s):%u(%s) ", arg->type, vmargType_toString(arg->type), arg->val, UserFuncs_get(arg->val)->id);
        break;
    case libfunc_a:
        fprintf(fout, "%d(%s):%u(%s) ", arg->type, vmargType_toString(arg->type), arg->val, LibFuncs_get(arg->val));
        break;
    case bool_a:
        fprintf(fout, "%d(%s):%u(%d) ", arg->type, vmargType_toString(arg->type), arg->val, arg->val);
        break;
    default:
        fprintf(fout, "%d(%s):%u(%s) ", arg->type, vmargType_toString(arg->type), arg->val, arg->var_name);
        break;
    }
}

static void print_vmarg(FILE *fout, vmarg *arg, int mode)
{
    if (!arg)
        return;

    if (arg->type == label_a)
    {
        if (mode)
            fprintf(fout, "%d(%s):%u ", arg->type, vmargType_toString(arg->type), arg->val);
        else
            fprintf(fout, "%d:%u ", arg->type, arg->val);
    }
    else if (arg->type == retval_a || arg->type == nil_a)
    {
        if (mode)
            fprintf(fout, "%d(%s): ", arg->type, vmargType_toString(arg->type));
        else
            fprintf(fout, "%d: ", arg->type);
    }
    else
    {
        if (arg->type != none_a)
        {
            if (mode)
                print_arg_value(fout, arg);
            else
                fprintf(fout, "%d:%u ", arg->type, arg->val);
        }
    }
}

void Instr_expand(void)
{
    assert(currInst == totalInst);
    Instruction *p = (Instruction *)malloc(NEW_SIZE_INSTR);
    if (instructions)
    {
        memcpy(p, instructions, CURR_SIZE_INSTR);
        free(instructions);
    }
    instructions = p;
    totalInst += EXPAND_SIZE_INSTR;
}

void Instr_emit(Instruction i)
{
    if (currInst == totalInst)
        Instr_expand();

    instructions[currInst].opcode = i.opcode;
    instructions[currInst].result = i.result;
    instructions[currInst].arg1 = i.arg1;
    instructions[currInst].arg2 = i.arg2;
    instructions[currInst].srcLine = i.srcLine;

    currInst++;
}

void Instr_display(FILE *fout, int mode)
{
    unsigned int i;
    fprintf(fout, "I:%u\n", currInst);
    for (i = 0; i < currInst; i++)
    {
        if (mode)
            fprintf(fout, "%u: ", i);
        fprintf(fout, "%s ", vmopcode_toString(instructions[i].opcode));
        print_vmarg(fout, instructions[i].result, mode);
        print_vmarg(fout, instructions[i].arg1, mode);
        print_vmarg(fout, instructions[i].arg2, mode);
        fprintf(fout, "line:%u", instructions[i].srcLine);
        fprintf(fout, "\n");
    }
}

void Instr_free(void)
{
    if (instructions)
    {
        unsigned int i;
        for (i = 0; i < currInst; i++)
        {
            if (instructions[i].result)
            {
                /*if (instructions[i].result->var_name)
                    free(instructions[i].result->var_name);*/
                free(instructions[i].result);
            }
            if (instructions[i].arg1)
            {
                /*if (instructions[i].arg1->var_name)
                    free(instructions[i].arg1->var_name);*/
                free(instructions[i].arg1);
            }
            if (instructions[i].arg2)
            {
                /*if (instructions[i].arg2->var_name)
                    free(instructions[i].arg2->var_name);*/
                free(instructions[i].arg2);
            }
        }
        free(instructions);
        instructions = (Instruction *)0;
    }
}

unsigned int Instr_nextInstr(void)
{
    return currInst;
}

/*IncompleteJump *IncJump_insert(unsigned int instrNo, unsigned int iaddress)
{
    IncompleteJump *ij = (IncompleteJump *)malloc(sizeof(struct IncompleteJump));
    ij->instrNo = instrNo;
    ij->iaddress = iaddress;
    ij->next = NULL;

    if (ij_head == NULL)
        ij_head = ij_last = ij;
    else
    {
        ij_last->next = ij;
        ij_last = ij;
    }

    ij_total++;

    return ij;
}

void IncJump_free(void)
{
    IncompleteJump *curr = ij_head;
    while (curr)
    {
        IncompleteJump *tmp = curr->next;
        free(curr);
        curr = tmp;
    }
}*/