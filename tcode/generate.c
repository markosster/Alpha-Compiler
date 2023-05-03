#include "generate.h"

OffsetStack *constTabFuncIndex_Stack = (OffsetStack *)NULL;

generator_t generators[] = {
    generate_JUMP,
    generate_ASSIGN,
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_UMINUS,
    generate_AND,
    generate_OR,
    generate_NOT,
    generate_NEWTABLE,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_IF_EQ,
    generate_IF_NOTEQ,
    generate_IF_GREATER,
    generate_IF_GREATEREQ,
    generate_IF_LESS,
    generate_IF_LESSEQ,
    generate_CALL,
    generate_PARAM,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_FUNCEND,
    generate_RETURN,
    generate_NOP,
};

void makeOperand(Expr *expr, vmarg *arg)
{
    if (!expr)
    {
        arg->val = -1;
        return;
    }

    switch (expr->type)
    {
    /* Variables */
    case var_e:
    case tableitem_e:
    case arithepxr_e:
    case boolexpr_e:
    case assignexpr_e:
    case newtable_e:
    {
        arg->val = expr->sym->offset;
        arg->var_name = (char *)expr->sym->name;
        switch (expr->sym->scopeSpace)
        {
        case program_var:
            arg->type = global_a;
            break;
        case function_local:
            arg->type = local_a;
            break;
        case formal_arg:
            arg->type = formal_a;
            break;
        default:
            assert(0);
        }
        break;
    }

    /* Constants */
    case constbool_e:
    {
        arg->type = bool_a;
        arg->val = expr->boolConst;
        break;
    }
    case conststring_e:
    {
        arg->type = string_a;
        arg->val = StrConsts_add((char *)expr->strConst);
        break;
    }
    case constnum_e:
    {
        arg->type = number_a;
        arg->val = NumConsts_add(expr->numConst);
        break;
    }

    /* Functions */
    case programfunc_e:
    {
        UserFunc uf;
        uf.address = expr->sym->value.funcVal->tcode_addr;
        uf.localSize = expr->sym->value.funcVal->total_locals;
        uf.args = expr->sym->value.funcVal->total_args;
        uf.id = (char*)expr->sym->name;

        arg->type = userfunc_a;
        arg->val = UserFuncs_add(uf);

        break;
    }
    case libraryfunc_e:
    {
        arg->type = libfunc_a;
        arg->val = LibFuncs_add((char *)expr->sym->name);
        break;
    }

    case nil_e:
    {
        arg->type = nil_a;
        break;
    }
    default:
        assert(0);
    }
}

void makeRetValOperand(vmarg *arg)
{
    arg->type = retval_a;
}

static Instruction newInstruction()
{
    Instruction i;

    i.result = (vmarg *)malloc(sizeof(struct vmarg));
    i.arg1 = (vmarg *)malloc(sizeof(struct vmarg));
    i.arg2 = (vmarg *)malloc(sizeof(struct vmarg));

    i.result->type = none_a;
    i.result->val = -1;
    i.result->var_name = NULL;
    i.arg1->type = none_a;
    i.arg1->val = -1;
    i.arg1->var_name = NULL;
    i.arg2->type = none_a;
    i.arg2->val = -1;
    i.arg2->var_name = NULL;
    
    return i;
}

static void generate_operation(vmopcode opcode, Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = opcode;
    i.srcLine = quad->line;
    quad->taddress = Instr_nextInstr();
    makeOperand(quad->result, i.result);
    makeOperand(quad->arg1, i.arg1);
    makeOperand(quad->arg2, i.arg2);
    Instr_emit(i);
}

static void generate_relational(vmopcode opcode, Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = opcode;
    i.srcLine = quad->line;
    /* result argument as a label in "jump" instructions */
    i.result->type = label_a;
    i.result->val = quad->label;
    /* ------------------------------------------------- */
    quad->taddress = Instr_nextInstr();
    makeOperand(quad->arg1, i.arg1);
    makeOperand(quad->arg2, i.arg2);
    Instr_emit(i);
}

/* ------- Arithmetic Expressions ------- */
void generate_ADD(Quad *quad) { generate_operation(add_v, quad); }
void generate_SUB(Quad *quad) { generate_operation(sub_v, quad); }
void generate_MUL(Quad *quad) { generate_operation(mul_v, quad); }
void generate_DIV(Quad *quad) { generate_operation(div_v, quad); }
void generate_MOD(Quad *quad) { generate_operation(mod_v, quad); }
void generate_UMINUS(Quad *quad)
{
    quad->arg2 = Expr_numConst(-1);
    generate_operation(mul_v, quad);
}
/* -------------------------------------- */

/* ------ Relational Expressions ------ */
void generate_JUMP(Quad *quad) { generate_relational(jump_v, quad); }
void generate_IF_EQ(Quad *quad) { generate_relational(jeq_v, quad); }
void generate_IF_NOTEQ(Quad *quad) { generate_relational(jne_v, quad); }
void generate_IF_GREATER(Quad *quad) { generate_relational(jgt_v, quad); }
void generate_IF_GREATEREQ(Quad *quad) { generate_relational(jge_v, quad); }
void generate_IF_LESS(Quad *quad) { generate_relational(jlt_v, quad); }
void generate_IF_LESSEQ(Quad *quad) { generate_relational(jle_v, quad); }
/* ------------------------------------ */

/* ------ Logical Expressions ------ */
void generate_NOT(Quad *quad)
{ /* we implemented "short-circuit evaluation" so the tcode will be the same with the icode, built by the Relational Operators above */
} /* it never calls this function because we never put "NOT" operator inside quad's table */
void generate_OR(Quad *quad)
{ /* we implemented "short-circuit evaluation" so the tcode will be the same with the icode, built by the Relational Operators above */
} /* it never calls this function because we never put "OR" operator inside quad's table */
void generate_AND(Quad *quad)
{ /* we implemented "short-circuit evaluation" so the tcode will be the same with the icode, built by the Relational Operators above */
} /* it never calls this function because we never put "AND" operator inside quad's table */
/* --------------------------------- */

/* ------ Assignments / Tables ------ */
void generate_NEWTABLE(Quad *quad) { generate_operation(newtable_v, quad); }
void generate_TABLEGETELEM(Quad *quad) { generate_operation(tablegetelem_v, quad); }
void generate_TABLESETELEM(Quad *quad) { generate_operation(tablesetelem_v, quad); }
void generate_ASSIGN(Quad *quad) { generate_operation(assign_v, quad); }
void generate_NOP(Quad *quad)
{
    Instruction i;
    i.opcode = nop_v;
    i.result = i.arg1 = i.arg2 = NULL;
    i.srcLine = 0;
    Instr_emit(i);
}
/* ---------------------------------- */

/* ------ Function Call ------ */
void generate_PARAM(Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = pusharg_v;
    i.srcLine = quad->line;
    quad->taddress = Instr_nextInstr();
    makeOperand(quad->result, i.result);
    Instr_emit(i);
}
void generate_CALL(Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = call_v;
    i.srcLine = quad->line;
    quad->taddress = Instr_nextInstr();
    makeOperand(quad->result, i.result);
    Instr_emit(i);
}
void generate_GETRETVAL(Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = assign_v;
    i.srcLine = quad->line;
    quad->taddress = Instr_nextInstr();
    makeOperand(quad->result, i.result);
    makeRetValOperand(i.arg1);
    Instr_emit(i);
}
/* --------------------------- */

/* ----- Function Definition ----- */
void generate_FUNCSTART(Quad *quad)
{
    Instruction i = newInstruction();

    /* Update func symbol info */
    SymTableEntry *f = quad->result->sym;
    f->value.funcVal->tcode_addr = Instr_nextInstr();

    i.opcode = funcenter_v;
    i.srcLine = quad->line;
    quad->taddress = Instr_nextInstr();

    OffsetStack_push(&constTabFuncIndex_Stack, currUserFuncs);

    makeOperand(quad->result, i.result);
    Instr_emit(i);
}
void generate_FUNCEND(Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = funcexit_v;
    i.srcLine = quad->line;
    quad->taddress = Instr_nextInstr();

    i.result->type = userfunc_a;
    i.result->val = OffsetStack_pop(&constTabFuncIndex_Stack);
    /*makeOperand(quad->result, i.result);*/

    Instr_emit(i);
}
void generate_RETURN(Quad *quad)
{
    Instruction i = newInstruction();
    i.opcode = assign_v;
    i.srcLine = quad->line;
    makeRetValOperand(i.result);
    makeOperand(quad->result, i.arg1);
    Instr_emit(i);
}
/* ------------------------------- */

void generate_tcode(void)
{
    unsigned int i;
    for (i = 0; i < currQuad; i++)
        (*generators[quads[i].op])(quads + i);
}