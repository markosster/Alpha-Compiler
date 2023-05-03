#ifndef EXPR_H
#define EXPR_H

#include "../symtable/symtable.h"

typedef enum ExprType
{
    var_e,
    tableitem_e,
    arithepxr_e,
    boolexpr_e,
    assignexpr_e,
    newtable_e,
    constbool_e,
    conststring_e,
    constnum_e,
    programfunc_e,
    libraryfunc_e,
    nil_e
} ExprType;

typedef struct Expr
{
    enum ExprType type;
    struct SymTableEntry *sym;
    /* expression's index */
    struct Expr *index;
    double numConst;
    const char *strConst;
    unsigned char boolConst;
    /* ------------------ */
    struct Expr *next;
    struct Expr *prev;
    int isNOTBoolOp;
    /* boolean lists */
    struct QuadList *trueList;
    struct QuadList *falseList;
    /* ------------- */
    struct Expr *nextInFreeList;
} Expr;

typedef struct F_call
{
    struct Expr *elist;
    unsigned char method;
    char *name;
} F_call;

typedef struct HashExprObject
{
    struct Expr *key;
    struct Expr *value;
    struct HashExprObject *next;
} HashExprObject;

extern Expr *exprFreeList_head;
extern Expr *exprFreeList_last;
void ExprFreeList_insert(Expr *expr);
void ExprFreeList_free(void);

void Expr_free(Expr **e);
Expr *Expr_new(ExprType etype, SymTableEntry *sym);
Expr *Expr_numConst(double num);
Expr *Expr_strConst(char **str);
Expr *Expr_boolConst(unsigned char bool);
Expr *Expr_lvalue(SymTableEntry *sym);

HashExprObject *HashExprObject_new(struct Expr *key, struct Expr *value);
void HashExprObject_free(HashExprObject *head);

#endif