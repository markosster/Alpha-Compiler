#include "memory.h"

avm_memcell stack[AVM_STACKSIZE];
avm_memcell ax, bx, cx;
avm_memcell retval;
unsigned top, topsp;
unsigned totalActuals;
unsigned totalGlobals;

char *typeStrings[] = {
    "number",
    "string",
    "bool",
    "table",
    "(userfunc)",
    "(libfunc)",
    "nil",
    "undef"};

void avm_memclear_string(avm_memcell *m)
{
    assert(m->data.strVal);
    free(m->data.strVal);
    m->data.strVal = NULL;
}
void avm_memclear_table(avm_memcell *m)
{
    assert(m->data.tableVal);
    avm_table_dec_refcounter(m->data.tableVal);
}

typedef void (*memclear_func_t)(avm_memcell *);
memclear_func_t memClearFuncs[] = {
    0,                   /* number */
    avm_memclear_string, /* string */
    0,                   /* bool */
    avm_memclear_table,  /* table */
    0,                   /* userfunc */
    0,                   /* libfunc */
    0,                   /* nil */
    0                    /* undef */
};

void avm_memcell_clear(avm_memcell *m)
{
    if (m->type != undef_m)
    {
        memclear_func_t f = memClearFuncs[m->type];
        if (f)
            (*f)(m);
        m->type = undef_m;
    }
}

/* to string */
char *number_tostring(avm_memcell *m)
{
    assert(m->type == number_m);
    char *str = float_tostr(m->data.numVal);
    return str;
}
char *string_tostring(avm_memcell *m)
{
    assert(m->type == string_m);
    return strdup(m->data.strVal);
}
char *bool_tostring(avm_memcell *m)
{
    assert(m->type == bool_m);
    if (m->data.boolVal)
        return strdup("true");
    return strdup("false");
}
char *table_tostring(avm_memcell *m)
{
    assert(m->type == table_m);
    avm_table_display(m->data.tableVal);
    return NULL;
}
char *userfunc_tostring(avm_memcell *m)
{
    assert(m->type == userfunc_m);
    char *funcname = strdup(m->data.userfuncVal->id);
    char *s = malloc(11 + strlen(m->data.userfuncVal->id));
    strcpy(s, "userfunc::");
    strcat(s, funcname);
    strcat(s, "\0");
    free(funcname);
    return s;
}
char *libfunc_tostring(avm_memcell *m)
{
    assert(m->type == libfunc_m);
    char *libfuncname = strdup(m->data.libfuncVal);
    char *s = malloc(10 + strlen(m->data.libfuncVal));
    strcpy(s, "libfunc::");
    strcat(s, libfuncname);
    strcat(s, "\0");
    free(libfuncname);
    return s;
}
char *nil_tostring(avm_memcell *m)
{
    assert(m->type == nil_m);
    return strdup("nil");
}
char *undef_tostring(avm_memcell *m)
{
    assert(m->type == undef_m);
    return strdup("undefined");
}

typedef char *(*tostring_func_t)(avm_memcell *);
tostring_func_t toStringFuncs[] = {
    number_tostring,
    string_tostring,
    bool_tostring,
    table_tostring,
    userfunc_tostring,
    libfunc_tostring,
    nil_tostring,
    undef_tostring,
};

char *avm_tostring(avm_memcell *m)
{
    assert(m->type >= 0 && m->type <= undef_m);
    return (*toStringFuncs[m->type])(m);
}

/* to bool */
unsigned char number_tobool(avm_memcell *m)
{
    return m->data.numVal != 0;
}
unsigned char string_tobool(avm_memcell *m)
{
    return m->data.strVal[0] != 0;
}
unsigned char bool_tobool(avm_memcell *m)
{
    return m->data.boolVal;
}
unsigned char table_tobool(avm_memcell *m)
{
    return 1;
}
unsigned char userfunc_tobool(avm_memcell *m)
{
    return 1;
}
unsigned char libfunc_tobool(avm_memcell *m)
{
    return 1;
}
unsigned char nil_tobool(avm_memcell *m)
{
    return 0;
}
unsigned char undef_tobool(avm_memcell *m)
{
    assert(0);
    return 0;
}

typedef unsigned char (*tobool_func_t)(avm_memcell *);
tobool_func_t toBoolFuncs[] = {
    number_tobool,
    string_tobool,
    bool_tobool,
    table_tobool,
    userfunc_tobool,
    libfunc_tobool,
    nil_tobool,
    undef_tobool};

unsigned char avm_tobool(avm_memcell *m)
{
    assert(m->type >= 0 && m->type < undef_m);
    return (*toBoolFuncs[m->type])(m);
}

void avm_dec_top(void)
{
    if (!top)
        AVM_ERROR("Stack overflow!")
    else
        --top;
}

void avm_push_envvalue(unsigned val)
{
    stack[top].mem_space = memspace_env;
    stack[top].type = number_m;
    stack[top].data.numVal = val;
    avm_dec_top();
}
void avm_callsave_env(void)
{
    avm_push_envvalue(totalActuals);
    avm_push_envvalue(pc + 1);
    avm_push_envvalue(top + totalActuals + 2);
    avm_push_envvalue(topsp);
}

unsigned avm_get_envvalue(unsigned i)
{
    assert(stack[i].type == number_m);
    unsigned val = (unsigned)stack[i].data.numVal;
    assert((unsigned)stack[i].data.numVal == (unsigned)val);
    return val;
}

void avm_memcell_assign(avm_memcell *lvalue, avm_memcell *rvalue)
{
    /* same memory cells */
    if (lvalue == rvalue)
        return;

    /* same tables, no need to assign */
    if (lvalue->type == table_m &&
        rvalue->type == table_m &&
        lvalue->data.tableVal == rvalue->data.tableVal)
        return;

    /* undefined rvalue content */
    if (rvalue->type == undef_m)
        AVM_WARNING("Assigning from undefined content!")

    /* clean up the memory cell for the new content to be stored */
    avm_memcell_clear(lvalue);

    /* memory copy from rvalue to lvalue */
    memcpy(lvalue, rvalue, sizeof(avm_memcell));

    /* take care of copied string values or reference counting in case of table */
    if (lvalue->type == string_m)
        lvalue->data.strVal = strdup(rvalue->data.strVal);
    else if (lvalue->type == table_m)
        avm_table_inc_refcounter(lvalue->data.tableVal);
}

unsigned avm_totalactuals(void)
{
    return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell *avm_get_actual(unsigned i)
{
    assert(i < avm_totalactuals());
    return &stack[topsp + AVM_NUMACTUALS_OFFSET + 1 + i];
}

avm_memcell *avm_translate_operand(vmarg *arg, avm_memcell *reg)
{
    switch (arg->type)
    {
        /* Variables */
    case global_a:
    {
        stack[AVM_STACKSIZE - 1 - arg->val].mem_space = memspace_global;
        return &stack[AVM_STACKSIZE - 1 - arg->val];
    }
    case local_a:
    {
        stack[topsp - arg->val].mem_space = memspace_funclocal;
        return &stack[topsp - arg->val];
    }
    case formal_a:
    {
        stack[topsp + AVM_STACKENV_SIZE + 1 + arg->val].mem_space = memspace_funcactual;
        return &stack[topsp + AVM_STACKENV_SIZE + 1 + arg->val];
    }
    case retval_a:
        return &retval;

        /* Constants */
    case number_a:
    {
        reg->type = number_m;
        reg->data.numVal = NumConsts_get(arg->val);
        return reg;
    }
    case string_a:
    {
        reg->type = string_m;
        reg->data.strVal = strdup(StrConsts_get(arg->val));
        return reg;
    }
    case bool_a:
    {
        reg->type = bool_m;
        reg->data.boolVal = arg->val;
        return reg;
    }

        /* Functions */
    case userfunc_a:
    {
        reg->type = userfunc_m;
        /*UserFunc uf;
        uf.address = UserFuncs_get(arg->val)->address;
        uf.id = strdup(UserFuncs_get(arg->val)->id);
        uf.localSize = UserFuncs_get(arg->val)->localSize;*/
        reg->data.userfuncVal = UserFuncs_get(arg->val);
        return reg;
    }
    case libfunc_a:
    {
        reg->type = libfunc_m;
        reg->data.libfuncVal = LibFuncs_get(arg->val);
        return reg;
    }

        /* Nil, Undefined */
    case nil_a:
    {
        reg->type = nil_m;
        return reg;
    }
    default:
        assert(0);
    }
}

void avm_print_memcell_value(FILE *fout, avm_memcell *m)
{
    if (m->type == table_m)
    {
        fprintf(fout, "(table)");
        return;
    }

    char *s = avm_tostring(m);
    if (s)
    {
        if (m->type == string_m)
            fprintf(fout, "\"%s\"", s);
        else
            fprintf(fout, "%s", s);
        free(s);
    }
}

void avm_mem_display(FILE *fout)
{
    unsigned i;

    fprintf(fout, "\n------------------ AVM MEMORY ------------------\n");

    fprintf(fout, "[retval] ->          [type: %s, value: ", typeStrings[retval.type]);
    avm_print_memcell_value(fout, &retval);
    fprintf(fout, "]\n");

    fprintf(fout, "[ax] ->              [type: %s, value: ", typeStrings[ax.type]);
    avm_print_memcell_value(fout, &ax);
    fprintf(fout, "]\n");

    fprintf(fout, "[bx] ->              [type: %s, value: ", typeStrings[bx.type]);
    avm_print_memcell_value(fout, &bx);
    fprintf(fout, "]\n");

    fprintf(fout, "[cx] ->              [type: %s, value: ", typeStrings[cx.type]);
    avm_print_memcell_value(fout, &cx);
    fprintf(fout, "]\n");

    fprintf(fout, "\n-------------------- STACK --------------------\n");

    for (i = AVM_STACKSIZE - 1; i >= top; i--)
    {
        if (top == i && topsp != i)
            fprintf(fout, "[TOP] ->       %u: [", i);
        else if (topsp == i && top != i)
            fprintf(fout, "[TOPSP] ->     %u: [", i);
        else if (top == i && topsp == i)
            fprintf(fout, "[TOP/TOPSP] -> %u: [", i);
        else
            fprintf(fout, "               %u: [", i);

        fprintf(fout, "memspace: ");
        switch (stack[i].mem_space)
        {
        case memspace_funcactual:
            fprintf(fout, "actual   ");
            break;
        case memspace_env:
            fprintf(fout, "env_saved");
            break;
        case memspace_funclocal:
            fprintf(fout, "local    ");
            break;
        case memspace_global:
            fprintf(fout, "global   ");
            break;
        default:
            fprintf(fout, "nonespace");
            break;
        }

        fprintf(fout, ", type: ");
        fprintf(fout, "%s", typeStrings[stack[i].type]);

        fprintf(fout, ", value: ");
        if (stack[i].type == table_m)
            fprintf(fout, "[table]");
        else
            avm_print_memcell_value(fout, &stack[i]);

        fprintf(fout, "]\n");
    }

    fprintf(fout, "-----------------------------------------------\n");
}

int avm_initialize()
{
    unsigned int i;

    top = topsp = AVM_STACKSIZE - 1 - totalGlobals;

    /* an insane case of total globals filling the whole stack... DO NOT DARE. :) */
    if ((int)top <= 0)
    {
        AVM_ERROR("Stack overflow!")
        return 0;
    }

    pc = 0;
    executionFinished = 0;
    totalActuals = 0;
    currLine = 0;

    for (i = 0; i < AVM_STACKSIZE; i++)
    {
        AVM_WIPEOUT(stack[i]);
        stack[i].type = undef_m;
        stack[i].mem_space = memspace_none;
    }

    avm_libfuncs_init();
    code = instructions;
    codeSize = currInst;

    return 1;
}

void avm_mem_free()
{
    unsigned i;

    Consts_free();
    Instr_free();
    avm_libfuncs_free();

    top = topsp = AVM_STACKSIZE - 1;
    pc = 0;
    executionFinished = 0;
    totalActuals = 0;
    currLine = 0;

    code = (Instruction *)NULL;
    codeSize = 0;

    /* stack */
    for (i = 0; i < AVM_STACKSIZE; i++)
    {
        if (stack[i].type != undef_m)
            avm_memcell_clear(&stack[i]);

        AVM_WIPEOUT(stack[i]);
        stack[i].type = undef_m;
        stack[i].mem_space = memspace_none;
    }

    /* registers */
    if (retval.type != undef_m)
        avm_memcell_clear(&retval);
    AVM_WIPEOUT(retval);
    retval.type = undef_m;
    retval.mem_space = memspace_none;

    if (ax.type != undef_m)
        avm_memcell_clear(&ax);
    AVM_WIPEOUT(ax);
    ax.type = undef_m;
    ax.mem_space = memspace_none;

    if (bx.type != undef_m)
        avm_memcell_clear(&bx);
    AVM_WIPEOUT(bx);
    bx.type = undef_m;
    bx.mem_space = memspace_none;

    if (cx.type != undef_m)
        avm_memcell_clear(&cx);
    AVM_WIPEOUT(cx);
    cx.type = undef_m;
    cx.mem_space = memspace_none;
}