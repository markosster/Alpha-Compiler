#include "execute.h"

Instruction *code;
unsigned char executionFinished;
unsigned pc;
unsigned currLine;
unsigned codeSize;

execute_func_t executor[] = {
    execute_ASSIGN,
    execute_ADD,
    execute_SUB,
    execute_MUL,
    execute_DIV,
    execute_MOD,
    execute_UMINUS,
    execute_AND,
    execute_OR,
    execute_NOT,
    execute_JEQ,
    execute_JNE,
    execute_JLE,
    execute_JGE,
    execute_JLT,
    execute_JGT,
    execute_CALL,
    execute_PUSHARG,
    execute_FUNCENTER,
    execute_FUNCEXIT,
    execute_NEWTABLE,
    execute_TABLEGETELEM,
    execute_TABLESETELEM,
    execute_JUMP,
    execute_NOP};

typedef double (*arithmetic_func_t)(double x, double y);
double add_impl(double x, double y) { return x + y; }
double sub_impl(double x, double y) { return x - y; }
double mul_impl(double x, double y) { return x * y; }
double div_impl(double x, double y)
{
    if (y == 0)
    {
        AVM_ERROR("Don't even try to divide by zero :)!")
        return -1;
    }
    return x / y;
}
double mod_impl(double x, double y)
{
    if (y == 0)
    {
        AVM_ERROR("Don't even try to divide by zero :)!")
        return -1;
    }
    return ((unsigned)x % (unsigned)y);
}

arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl};

typedef unsigned char (*cmp_func_t)(double x, double y);
unsigned char jge_impl(double x, double y) { return x >= y; }
unsigned char jgt_impl(double x, double y) { return x > y; }
unsigned char jle_impl(double x, double y) { return x <= y; }
unsigned char jlt_impl(double x, double y) { return x < y; }

cmp_func_t comparisonFuncs[] = {
    jle_impl,
    jge_impl,
    jlt_impl,
    jgt_impl};

static void execute_eq(Instruction *i, unsigned char eq_mode)
{
    assert(i->result->type == label_a);

    avm_memcell *rv1 = avm_translate_operand(i->arg1, &ax);
    avm_memcell *rv2 = avm_translate_operand(i->arg2, &bx);

    unsigned char result = 0;

    if (rv1->type == undef_m || rv2->type == undef_m)
        AVM_ERROR("'Undefined' involved in equality!")
    else if (rv1->type == nil_m || rv2->type == nil_m)
        result = eq_mode ? (rv1->type == nil_m && rv2->type == nil_m) : (rv1->type != nil_m && rv2->type != nil_m);
    else if (rv1->type == bool_m || rv2->type == bool_m)
        result = eq_mode ? (avm_tobool(rv1) == avm_tobool(rv2)) : (avm_tobool(rv1) != avm_tobool(rv2));
    else if (rv1->type != rv2->type)
        AVM_ERROR("'%s == %s' is illegal!", typeStrings[rv1->type], typeStrings[rv2->type])
    else
    {
        assert(rv1->type == rv2->type);
        switch (rv1->type)
        {
        case number_m:
        {
            result = eq_mode ? (rv1->data.numVal == rv2->data.numVal) : (rv1->data.numVal != rv2->data.numVal);
            break;
        }
        case string_m:
        {
            result = eq_mode ? (!strcmp(rv1->data.strVal, rv2->data.strVal)) : (strcmp(rv1->data.strVal, rv2->data.strVal));
            break;
        }
        case table_m:
        case userfunc_m:
        case libfunc_m:
        {
            result = eq_mode;
            break;
        }
        case nil_m:
        case undef_m:
        default:
            assert(0);
        }
    }

    if (!executionFinished && result)
        pc = i->result->val;
}

static void execute_relational(Instruction *i)
{
    assert(i->result->type == label_a);

    avm_memcell *rv1 = avm_translate_operand(i->arg1, &ax);
    avm_memcell *rv2 = avm_translate_operand(i->arg2, &bx);

    assert(rv1 && rv2);

    unsigned char result = 0;

    cmp_func_t relop = comparisonFuncs[i->opcode - jle_v];
    if (rv1->type != number_m || rv2->type != number_m)
    {
        /* convert boolean values (true, false) to 1 and 0 */
        /*if ((rv1->type == bool_m && rv2->type == number_m))
            result = (*relop)((double)rv1->data.boolVal, rv2->data.numVal);
        else if (rv1->type == number_m && rv2->type == bool_m)
            result = (*relop)(rv1->data.numVal, (double)rv2->data.boolVal);
        else if (rv1->type == bool_m && rv2->type == bool_m)
            result = (*relop)((double)rv1->data.boolVal, (double)rv2->data.boolVal);
        else*/
        {
            AVM_ERROR("Comparison with non-number!")
            return;
        }
    }
    else
        result = (*relop)(rv1->data.numVal, rv2->data.numVal);

    if (!executionFinished && result)
        pc = i->result->val;
}

static void execute_arithmetic(Instruction *i)
{
    avm_memcell *lv = avm_translate_operand(i->result, (avm_memcell *)0);
    avm_memcell *rv1 = avm_translate_operand(i->arg1, &ax);
    avm_memcell *rv2 = avm_translate_operand(i->arg2, &bx);

    assert(lv);
    assert(&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval);
    assert(rv1 && rv2);

    if (rv1->type != number_m || rv2->type != number_m)
    {
        /* string concatenation */
        if (rv1->type == string_m && rv2->type == string_m)
        {
            char *s = malloc(strlen(rv1->data.strVal) + strlen(rv2->data.strVal));
            memset(s, 0, strlen(s) * sizeof(char));
            avm_memcell_clear(lv);
            lv->type = string_m;
            strcpy(s, rv1->data.strVal);
            strcat(s, rv2->data.strVal);
            lv->data.strVal = s;
        }
        else
            AVM_ERROR("Not a number in arithmetic operation!")
    }
    else
    {
        arithmetic_func_t arithop = arithmeticFuncs[i->opcode - add_v];
        avm_memcell_clear(lv);
        lv->type = number_m;
        lv->data.numVal = (*arithop)(rv1->data.numVal, rv2->data.numVal);
    }
}

void execute_ASSIGN(Instruction *instr)
{
    avm_memcell *lv = avm_translate_operand(instr->result, (avm_memcell *)0);
    avm_memcell *rv = avm_translate_operand(instr->arg1, &ax);

    assert(lv && (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval));
    assert(rv && (&stack[AVM_STACKSIZE - 1] >= rv && rv > &stack[top] || rv == &retval || rv == &ax));

    avm_memcell_assign(lv, rv);
}

void execute_ADD(Instruction *i) { execute_arithmetic(i); }
void execute_SUB(Instruction *i) { execute_arithmetic(i); }
void execute_MUL(Instruction *i) { execute_arithmetic(i); }
void execute_DIV(Instruction *i) { execute_arithmetic(i); }
void execute_MOD(Instruction *i) { execute_arithmetic(i); }

/* ----- Unnecessary (Skip) ----- */
void execute_UMINUS(Instruction *i) {}
void execute_AND(Instruction *i) {}
void execute_OR(Instruction *i) {}
void execute_NOT(Instruction *i) {}
void execute_NOP(Instruction *i) {}
/* ------------------------------ */

void execute_JUMP(Instruction *i)
{
    assert(i->result->type == label_a);
    assert(i->result->val > 0 && i->result->val <= codeSize);

    if (!executionFinished)
        pc = i->result->val;
}

void execute_JEQ(Instruction *i) { execute_eq(i, 1); }
void execute_JNE(Instruction *i) { execute_eq(i, 0); }

void execute_JLE(Instruction *i) { execute_relational(i); }
void execute_JGE(Instruction *i) { execute_relational(i); }
void execute_JLT(Instruction *i) { execute_relational(i); }
void execute_JGT(Instruction *i) { execute_relational(i); }

void execute_CALL(Instruction *i)
{
    avm_memcell *func = avm_translate_operand(i->result, &ax);
    assert(func);

    if (func->type == table_m)
    {
        Instruction tmp_i;
        tmp_i.result = i->result;
        tmp_i.srcLine = i->srcLine;
        tmp_i.opcode = pusharg_v;
        execute_PUSHARG(&tmp_i);
    }
    avm_callsave_env();

    switch (func->type)
    {
    case table_m:
    {
        func = avm_table_functor(func);
    }
    case userfunc_m:
    {
        if (func)
        {
            pc = func->data.userfuncVal->address;
            assert(pc < AVM_ENDING_PC);
            assert(code[pc].opcode == funcenter_v);
        }
        break;
    }
    case string_m:
    {
        avm_call_libfunc(func->data.strVal);
        break;
    }
    case libfunc_m:
    {
        avm_call_libfunc(func->data.libfuncVal);
        break;
    }
    default:
    {
        char *s = avm_tostring(func);
        AVM_ERROR("Call: cannot bind '%s' to function!", s)
        free(s);
    }
    }
}

void execute_PUSHARG(Instruction *i)
{
    avm_memcell *arg = avm_translate_operand(i->result, &ax);
    assert(arg);
    stack[top].mem_space = memspace_funcactual;
    avm_memcell_assign(&stack[top], arg);
    ++totalActuals;
    avm_dec_top();
}
void execute_FUNCENTER(Instruction *i)
{
    avm_memcell *func = avm_translate_operand(i->result, &ax);
    assert(func);
    assert(pc == func->data.userfuncVal->address);

    totalActuals = 0;
    topsp = top;
    top = top - func->data.userfuncVal->localSize;

    if (avm_totalactuals() < func->data.userfuncVal->args)
        AVM_ERROR("Out of stack (too few arguments in function call)!")

    if ((int)top <= 0)
        AVM_ERROR("Stack overflow!")
}
void execute_FUNCEXIT(Instruction *i)
{
    unsigned oldTop = top;
    top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
    pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
    topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);

    while (++oldTop <= top)
        avm_memcell_clear(&stack[oldTop]);
}

void execute_NEWTABLE(Instruction *i)
{
    avm_memcell *lv = avm_translate_operand(i->result, (avm_memcell *)0);

    assert(lv && (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval));
    avm_memcell_clear(lv);

    lv->type = table_m;
    lv->data.tableVal = avm_table_new();
    avm_table_inc_refcounter(lv->data.tableVal);
}
void execute_TABLEGETELEM(Instruction *i)
{
    avm_memcell *lv = avm_translate_operand(i->result, (avm_memcell *)0);
    avm_memcell *t = avm_translate_operand(i->arg1, (avm_memcell *)0);
    avm_memcell *index = avm_translate_operand(i->arg2, &ax);

    assert(lv && (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top] || lv == &retval));
    assert(t && (&stack[AVM_STACKSIZE - 1] >= t && t > &stack[top]));
    assert(index);

    avm_memcell_clear(lv);
    lv->type = nil_m;

    if (t->type != table_m)
        AVM_ERROR("Illegal use of type '%s' as table!", typeStrings[t->type])
    else
    {
        avm_memcell *content = avm_table_getelem(t->data.tableVal, index);
        if (content)
            avm_memcell_assign(lv, content);
        else
        {
            if (!tableSetElemNil)
            {
                char *is = avm_tostring(index);
                AVM_WARNING("Table element with index '%s' not found!", is)
                free(is);
            }
            tableSetElemNil = 0;
        }
    }
}
void execute_TABLESETELEM(Instruction *i)
{
    avm_memcell *t = avm_translate_operand(i->arg1, (avm_memcell *)0);
    avm_memcell *index = avm_translate_operand(i->arg2, &ax);
    avm_memcell *rv = avm_translate_operand(i->result, &bx);

    assert(t && (&stack[AVM_STACKSIZE - 1] >= t && t > &stack[top]));
    assert(index && rv);

    if (t->type != table_m)
        AVM_ERROR("Illegal use of type '%s' as table!", typeStrings[t->type])
    else
        avm_table_setelem(t->data.tableVal, index, rv);
}

void execute_cycle(void)
{
    if (executionFinished)
        return;
    else if (pc == AVM_ENDING_PC)
    {
        executionFinished = 1;
        return;
    }
    else
    {
        assert(pc < AVM_ENDING_PC);
        Instruction *instr = code + pc;
        assert(instr->opcode >= 0 && instr->opcode <= AVM_MAX_INSTRUCTIONS);
        if (instr->srcLine)
            currLine = instr->srcLine;
        unsigned oldpc = pc;
        (*executor[instr->opcode])(instr);
        // printf("OPCODE: %s\n", vmopcode_toString(instr->opcode));
        if (pc == oldpc)
            ++pc;
    }
}
