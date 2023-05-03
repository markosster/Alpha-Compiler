#include "expr.h"

Expr *exprFreeList_head = NULL;
Expr *exprFreeList_last = NULL;

void ExprFreeList_insert(Expr *expr)
{
    if (exprFreeList_head == NULL)
    {
        exprFreeList_head = expr;
        exprFreeList_last = expr;
    }
    else
    {
        exprFreeList_last->nextInFreeList = expr;
        exprFreeList_last = expr;
    }
}

void ExprFreeList_free(void)
{
    Expr *curr = exprFreeList_head;

    while (curr)
    {
        Expr *tmp = curr->nextInFreeList;
        if (curr->strConst)
        {
            free((char *)curr->strConst);
            curr->strConst = (char *)NULL;
        }
        free(curr);
        curr = tmp;
    }
}

static const char *exprType_toString(ExprType exprType)
{
    switch (exprType)
    {
    case var_e:
        return "VARIABLE";
    case tableitem_e:
        return "TABLEITEM";
    case programfunc_e:
        return "PROGRAMFUNC";
    case libraryfunc_e:
        return "LIBRARYFUNC";
    case arithepxr_e:
        return "ARITHEXPR";
    case boolexpr_e:
        return "BOOLEXPR";
    case assignexpr_e:
        return "ASSIGNEXPR";
    case newtable_e:
        return "NEWTABLE";
    case constnum_e:
        return "CONSTNUM";
    case constbool_e:
        return "CONSTBOOL";
    case conststring_e:
        return "CONSTSTRING";
    case nil_e:
        return "NIL";
    }
}

void Expr_free(Expr **e)
{
    if (*e)
    {
        if ((*e)->strConst)
        {
            free((char *)(*e)->strConst);
            (*e)->strConst = (char *)NULL;
        }

        (*e)->index = (Expr *)NULL;
        (*e)->trueList = (QuadList *)NULL;
        (*e)->falseList = (QuadList *)NULL;
        (*e) = (Expr *)NULL;
    }
}

Expr *Expr_new(ExprType etype, SymTableEntry *sym)
{
    Expr *e = (Expr *)malloc(sizeof(struct Expr));
    memset(e, 0, sizeof(struct Expr));
    e->type = etype;
    e->next = (Expr *)NULL;
    e->prev = (Expr *)NULL;
    e->index = (Expr *)NULL;
    e->numConst = 0;
    e->strConst = (char *)NULL;
    e->boolConst = 0;
    e->sym = sym;
    e->trueList = (QuadList *)NULL;
    e->falseList = (QuadList *)NULL;
    e->isNOTBoolOp = 0;
    e->nextInFreeList = NULL;

    ExprFreeList_insert(e);

    return e;
}

Expr *Expr_numConst(double num)
{
    Expr *e = Expr_new(constnum_e, NULL);
    e->numConst = num;
    return e;
}

Expr *Expr_strConst(char **str)
{
    Expr *e = Expr_new(conststring_e, NULL);
    e->strConst = strdup(*str);
    free(*str);
    *str = (char *)NULL;
    return e;
}

Expr *Expr_boolConst(unsigned char bool)
{
    assert(bool == 0 || bool == 1);
    Expr *e = Expr_new(constbool_e, NULL);
    e->boolConst = !!bool;
    return e;
}

Expr *Expr_lvalue(SymTableEntry *sym)
{
    assert(sym);
    Expr *e = Expr_new(constnum_e, sym);

    switch (sym->type)
    {
    case GLOBAL:
    case LOCAL:
    case FORMAL:
        e->type = var_e;
        break;
    case USERFUNC:
        e->type = programfunc_e;
        break;
    case LIBFUNC:
        e->type = libraryfunc_e;
        break;
    default:
        assert(0);
    }
    return e;
}

HashExprObject *HashExprObject_new(Expr *key, Expr *value)
{
    HashExprObject *o = (HashExprObject *)malloc(sizeof(struct HashExprObject));
    o->key = key;
    o->value = value;
    o->next = NULL;
    return o;
}

void HashExprObject_free(HashExprObject *head)
{
    HashExprObject *curr = head, *tmp;
    while (curr)
    {
        tmp = curr->next;
        free(curr);
        curr = tmp;
    }
}