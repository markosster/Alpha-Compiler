#ifndef MANAGE_C
#define MANAGE_C

#include "symtable/symtable.h"
#include "symtable/funcstack.h"
#include "icode/quads.h"
#include "tcode/generate.h"

extern int yylineno;

/* hashtable for symbols */
SymTable *symtable = (SymTable *)NULL;
/* Stack to save loopcounter */
OffsetStack *loopCounter_Stack = (OffsetStack *)NULL;
/* Stack to save offsets */
OffsetStack *offset_Stack = (OffsetStack *)NULL;
/* Stack to save jump label before functions */
OffsetStack *jumpFunc_Stack = (OffsetStack *)NULL;
/* Stack to save current function to patch the jump labels for all the "return" statements */
FuncStack *currFunc_Stack = (FuncStack *)NULL;
/* counter for current scope, set to global scope (scope #0) */
unsigned int scope = 0;
/* counter for anonymous functions */
int anonymFuncCounter = 0;
/* counter for checking if we are inside a loop */
int loopCounter = 0;
/* counter for checking if we are inside a function */
int insideFuncCounter = 0;
/* flag used when we create a hashtable for an object and we want to initialize the head pointer of the list */
int insideIndexed = 0;
/* flag used for icode generation status */
int icode_status = 1;
int totalFuncArgs = 0;

#define COMPILE_ERROR(fout, ...)                                  \
    {                                                             \
        PRINT_RED(fout, "COMPILE ERROR in line %d => ", yylineno) \
        PRINT_RED(fout, __VA_ARGS__)                              \
        fprintf(fout, "\n");                                      \
        icode_status = 0;                                         \
    }

#define PRINT_OK(fout, ...)                \
    {                                      \
        /*PRINT_GREEN(fout, __VA_ARGS__)*/ \
        /*fprintf(fout, "\n");*/           \
    }

/* ------------------------------------------------------------- */
/* ---------------------- OTHER FUNCTIONS ---------------------- */
/* ------------------------------------------------------------- */

/* _____ WRITTING IN FILES _____ */
void write_quads(FILE *quads_file)
{
    Quads_okeanos_display(quads_file);
}

void write_binary(FILE *binary_file)
{
    fprintf(binary_file, "40434099\n");
    fprintf(binary_file, "G:%u\n", globals);
    Consts_display(binary_file);
    Instr_display(binary_file, 0);
}

void write_binary_text(FILE *text_file)
{
    fprintf(text_file, "40434099\n");
    fprintf(text_file, "G:%u\n", globals);
    Consts_display(text_file);
    Instr_display(text_file, 1);
}
/* _____________________________ */

void print_tcode_status()
{
    PRINT_CYAN(stderr, "=> t-code GENERATION: ");
    if (icode_status)
        PRINT_GREEN(stderr, "[SUCCESS]\n\n")
    else
        PRINT_RED(stderr, "[FAILED]\n\n")
}

void print_icode_status()
{
    PRINT_CYAN(stderr, "\n=> i-code GENERATION: ");
    if (icode_status)
        PRINT_GREEN(stderr, "[SUCCESS]\n")
    else
        PRINT_RED(stderr, "[FAILED]\n")
}

/* check if the given symbol name (id) is shadowing any of the library functions */
SymTableEntry *check_libfunc(const char *id)
{
    /* get the global scope (header list for scope#0) */
    ScopeListEntry *globalScopeEntry = ScopeList_get(symtable->scopeList, 0);

    if (globalScopeEntry)
    {
        SymTableEntry *libFuncEntry = globalScopeEntry->headScopeSymEntry;
        while (libFuncEntry)
        {
            /* found colission */
            if (libFuncEntry->type == LIBFUNC && !strcmp(libFuncEntry->name, id))
                return libFuncEntry;

            libFuncEntry = libFuncEntry->nextInScopeList;
        }
    }
    return NULL;
}

int check_funcid(Expr *expr)
{
    if (expr)
    {
        if (expr->type == programfunc_e || expr->type == libraryfunc_e)
            COMPILE_ERROR(stderr, "Illegal operation with function id!")
    }
    else
        return 0;

    if (!icode_status)
        return 0;
    return 1;
}

int check_arith(Expr *expr)
{
    if (expr)
    {
        if (check_funcid(expr))
            ;
        else
        {
            if (expr->type == constbool_e)
                COMPILE_ERROR(stderr, "Illegal arithmetic operation with boolean constant!")
            else if (expr->type == conststring_e)
                COMPILE_ERROR(stderr, "Illegal arithmetic operation with string constant!")
            else if (expr->type == nil_e)
                COMPILE_ERROR(stderr, "Illegal arithmetic operation with nil expression!")
            else if (expr->type == newtable_e)
                COMPILE_ERROR(stderr, "Illegal arithmetic operation with table!")
            else if (expr->type == boolexpr_e)
                COMPILE_ERROR(stderr, "Illegal arithmetic operation with boolean expression!")
        }
    }
    else
        return 0;

    if (!icode_status)
        return 0;
    return 1;
}

Expr *emit_ifboolexpr(Expr *expr)
{
    if (expr && expr->type == boolexpr_e)
    {
        Expr *boolExpr = Expr_new(var_e, Quads_newTemp(symtable, scope));
        Quads_emit(assign, boolExpr, Expr_boolConst(1), NULL, yylineno);
        Quads_emit(jump, NULL, NULL, NULL, yylineno);
        Quads_patchLabel(Quads_nextQuad() - 1, Quads_nextQuad() + 1);
        Quads_emit(assign, boolExpr, Expr_boolConst(0), NULL, yylineno);
        QuadList_backpatch(expr->trueList, Quads_nextQuad() - 3);
        QuadList_backpatch(expr->falseList, Quads_nextQuad() - 1);
        return boolExpr;
    }
    return expr;
}

int check_rel(Expr **expr, iopcode op)
{
    if ((*expr))
    {
        if ((*expr)->type == boolexpr_e)
        {
            if (op != if_eq && op != if_noteq)
                COMPILE_ERROR(stderr, "Operation with boolean expression!")
            else
            {
                if ((*expr)->isNOTBoolOp)
                    (*expr) = emit_ifboolexpr((*expr));
            }
        }
    }
    else
        return 0;

    if (!icode_status)
        return 0;
    return 1;
}

void update_scopeSpace_offset(SymTableEntry *sym)
{
    if (sym)
    {
        sym->scopeSpace = currScopeSpace();
        sym->offset = currScopeOffset();
        inCurrScopeOffset();
    }
}

Expr *emit_iftableitem(Expr *expr)
{
    if (expr)
    {
        if (expr->type != tableitem_e)
            return expr;
        else
        {
            Expr *result = Expr_new(var_e, Quads_newTemp(symtable, scope));
            Quads_emit(tablegetelem, result, expr, expr->index, yylineno);
            return result;
        }
    }
    return NULL;
}

Expr *member_item(Expr *lval, Expr *index)
{
    if (lval)
    {
        lval = emit_iftableitem(lval);
        Expr *ti = Expr_new(tableitem_e, lval->sym);
        ti->index = index;
        return ti;
    }
    return NULL;
}

Expr *make_call(Expr *lval, Expr *reversed_elist)
{
    Expr *func = emit_iftableitem(lval);
    Expr *currExpr = reversed_elist;
    Expr *result = NULL;

    while (currExpr)
    {
        Quads_emit(param, currExpr, NULL, NULL, yylineno);
        currExpr = currExpr->next;
    }

    Quads_emit(call, func, NULL, NULL, yylineno);
    result = Expr_new(var_e, Quads_newTemp(symtable, scope));
    Quads_emit(getretval, result, NULL, NULL, yylineno);
    return result;
}

void true_test(Expr **expr)
{
    if (expr && ((*expr)->type != boolexpr_e))
    {
        (*expr)->trueList = QuadList_new(Quads_nextQuad());
        (*expr)->falseList = QuadList_new(Quads_nextQuad() + 1);
        Quads_emit(if_eq, NULL, (*expr), Expr_boolConst(1), yylineno);
        Quads_emit(jump, NULL, NULL, NULL, yylineno);
    }
}

/* ------------------------------------------------------------------ */
/* ------------------ MANAGER FUNCTIONS FOR ACTIONS ----------------- */
/* ------------------------------------------------------------------ */

/* short-circuit evaluation */
unsigned int manage_S()
{
    return Quads_nextQuad();
}

unsigned int manage_M()
{
    return Quads_nextQuad();
}

unsigned int manage_N()
{
    Quads_emit(jump, NULL, NULL, NULL, yylineno);
    return Quads_nextQuad() - 1;
}

forprefix_s manage_forprefix(forprefix_s forprefix, unsigned int M, Expr *expr)
{
    forprefix.test = M;
    forprefix.enter = Quads_nextQuad();
    Quads_emit(if_eq, NULL, expr, Expr_boolConst(1), yylineno);
    return forprefix;
}

stmt manage_forstmt(forprefix_s forprefix, unsigned int N1, unsigned int N2, stmt *loopbody, unsigned int N3)
{
    Quads_patchLabel(forprefix.enter, N2 + 1); /* true jump */
    Quads_patchLabel(N1, Quads_nextQuad());    /* false jump */
    Quads_patchLabel(N2, forprefix.test);      /* loop jump */
    Quads_patchLabel(N3, N1 + 1);              /* closure jump */
    /* patch lists for break and continue */
    QuadList_backpatch(loopbody->breakList, Quads_nextQuad());
    QuadList_backpatch(loopbody->contList, N1 + 1);

    //QuadList_free(&loopbody->breakList);
    loopbody->breakList = NULL;
    //QuadList_free(&loopbody->contList);
    loopbody->contList = NULL;

    Stmt_makeList(loopbody); /* reset lists */
    return *loopbody;
}

stmt manage_stmts(stmt ss, stmt statement, stmt statements)
{
    /* merge stmt lists */
    ss.breakList = QuadList_merge(statement.breakList, statements.breakList);
    ss.contList = QuadList_merge(statement.contList, statements.contList);
    return ss;
}

stmt manage_break(stmt brk)
{
    Stmt_makeList(&brk);
    if (loopCounter == 0)
        COMPILE_ERROR(stderr, "Use of \"break\" statement while not in a loop!")
    else
    {
        brk.breakList = QuadList_new(Quads_nextQuad());
        Quads_emit(jump, NULL, NULL, NULL, yylineno);
    }
    return brk;
}

stmt manage_continue(stmt cont)
{
    Stmt_makeList(&cont);
    if (loopCounter == 0)
        COMPILE_ERROR(stderr, "Use of \"continue\" statement while not in a loop!")
    else
    {
        cont.contList = QuadList_new(Quads_nextQuad());
        Quads_emit(jump, NULL, NULL, NULL, yylineno);
    }
    return cont;
}

unsigned int manage_whilestart()
{
    return Quads_nextQuad();
}

stmt manage_whilestmt(unsigned int whilestart, unsigned int whilecond, stmt *loopbody)
{
    Quads_emit(jump, NULL, NULL, NULL, yylineno);
    Quads_patchLabel(Quads_nextQuad() - 1, whilestart);
    Quads_patchLabel(whilecond, Quads_nextQuad());

    /* patch lists for break and continue */
    QuadList_backpatch(loopbody->breakList, Quads_nextQuad());
    QuadList_backpatch(loopbody->contList, whilestart);

    //QuadList_free(&loopbody->breakList);
    loopbody->breakList = NULL;
    //QuadList_free(&loopbody->contList);
    loopbody->contList = NULL;

    Stmt_makeList(loopbody); /* reset lists */
    return *loopbody;
}

unsigned int manage_whilecond(Expr *expr)
{
    Quads_emit(if_eq, NULL, expr, Expr_boolConst(1), yylineno);
    Quads_patchLabel(Quads_nextQuad() - 1, Quads_nextQuad() + 1);
    Quads_emit(jump, NULL, NULL, NULL, yylineno);
    return Quads_nextQuad() - 1;
}

unsigned int manage_ifprefix(Expr *expr)
{
    Quads_emit(if_eq, NULL, expr, Expr_boolConst(1), yylineno);
    Quads_patchLabel(Quads_nextQuad() - 1, Quads_nextQuad() + 1);
    Quads_emit(jump, NULL, NULL, NULL, yylineno);
    return Quads_nextQuad() - 1; /* return last emitted quad to patch it later (after stmt) */
}

stmt manage_ifprefix_stmt(unsigned int ifprefix, stmt s)
{
    Quads_patchLabel(ifprefix, Quads_nextQuad());
    return s;
}

unsigned int manage_elseprefix()
{
    Quads_emit(jump, NULL, NULL, NULL, yylineno);
    return Quads_nextQuad() - 1;
}

stmt manage_ifelseprefix_stmt(stmt ss, unsigned int ifprefix, stmt s2, unsigned int elseprefix, stmt s4)
{
    Quads_patchLabel(ifprefix, elseprefix + 1);
    Quads_patchLabel(elseprefix, Quads_nextQuad());
    /* merge stmt lists */
    ss.breakList = QuadList_merge(s2.breakList, s4.breakList);
    ss.contList = QuadList_merge(s2.contList, s4.contList);
    return ss;
}

void manage_indexed(HashExprObject *retindexed, HashExprObject *indexedelem, HashExprObject *indexed)
{
    if (!insideIndexed)
        retindexed = indexedelem;
    retindexed->next = indexed;
}

Expr *manage_term_prefix(Expr *expr, iopcode op)
{
    assert(op == add || op == sub || op == uminus);
    Expr *term = NULL, *one = NULL;

    if (check_arith(expr))
    {
        if (op == uminus)
        {
            term = Expr_new(var_e, NULL);
            /* temporary var reuse when in r-value */
            term->sym = Quads_isTempVar(expr) ? expr->sym : Quads_newTemp(symtable, scope);
            Quads_emit(uminus, term, expr, NULL, yylineno);
        }
        else
        {
            one = Expr_numConst(1);
            if (expr->type == tableitem_e)
            {
                term = emit_iftableitem(expr);
                Quads_emit(op, term, term, one, yylineno);
                Quads_emit(tablesetelem, term, expr, expr->index, yylineno);
            }
            else
            {
                term = Expr_new(var_e, Quads_newTemp(symtable, scope));
                Quads_emit(op, expr, expr, one, yylineno);
                Quads_emit(assign, term, expr, NULL, yylineno);
            }
        }
    }

    return term;
}

Expr *manage_term_postfix(Expr *expr, iopcode op)
{
    assert(op == add || op == sub);
    Expr *term = NULL, *one = NULL;

    one = Expr_numConst(1);
    term = Expr_new(var_e, Quads_newTemp(symtable, scope));

    if (check_arith(expr))
    {
        if (expr->type == tableitem_e)
        {
            Expr *item_val = emit_iftableitem(expr);
            Quads_emit(assign, term, item_val, NULL, yylineno);
            Quads_emit(op, item_val, item_val, one, yylineno);
            Quads_emit(tablesetelem, item_val, expr, expr->index, yylineno);
        }
        else
        {
            Quads_emit(assign, term, expr, NULL, yylineno);
            Quads_emit(op, expr, expr, one, yylineno);
        }
    }

    return term;
}

Expr *manage_expr_arithop_expr(Expr *left_expr, iopcode arithop, Expr *right_expr)
{
    Expr *expr = Expr_new(arithepxr_e, NULL);

    if (check_arith(left_expr) && check_arith(right_expr))
    {
        /*if (left_expr->type == constnum_e && right_expr->type == constnum_e)
        {
            expr->type = constnum_e;
            expr->numConst = left_expr->numConst + right_expr->numConst;
        }
        else
        {*/
        /* temp reuse */
        if (Quads_isTempVar(left_expr))
            expr->sym = left_expr->sym;
        else if (Quads_isTempVar(right_expr))
            expr->sym = right_expr->sym;
        else
            expr->sym = Quads_newTemp(symtable, scope);
        Quads_emit(arithop, expr, left_expr, right_expr, yylineno);
        /*}*/
    }

    return expr;
}

Expr *manage_expr_relop_expr(Expr *left_expr, iopcode relop, Expr *right_expr)
{
    Expr *expr = Expr_new(boolexpr_e, NULL);

    if (check_funcid(left_expr) &&
        check_funcid(right_expr) &&
        check_rel(&left_expr, relop) &&
        check_rel(&right_expr, relop))
    {
        expr->trueList = QuadList_new(Quads_nextQuad());
        expr->falseList = QuadList_new(Quads_nextQuad() + 1);
        Quads_emit(relop, NULL, left_expr, right_expr, yylineno);
        Quads_emit(jump, NULL, NULL, NULL, yylineno);
    }

    return expr;
}

Expr *manage_expr_boolop_expr(Expr *left_expr, iopcode boolop, unsigned int S_quad, Expr *right_expr)
{
    assert(boolop == not || boolop == and || boolop == or);
    Expr *expr = Expr_new(boolexpr_e, NULL);

    if (right_expr)
    {
        true_test(&right_expr);

        if (boolop == not )
        {
            expr->isNOTBoolOp = 1;
            expr->trueList = right_expr->falseList;
            expr->falseList = right_expr->trueList;
        }
        else
        {
            if (left_expr)
            {
                if (boolop == and)
                {
                    QuadList_backpatch(left_expr->trueList, S_quad);
                    expr->trueList = right_expr->trueList;
                    expr->falseList = QuadList_merge(left_expr->falseList, right_expr->falseList);
                }
                else
                {
                    QuadList_backpatch(left_expr->falseList, S_quad);
                    expr->trueList = QuadList_merge(left_expr->trueList, right_expr->trueList);
                    expr->falseList = right_expr->falseList;
                }
            }
        }
    }

    return expr;
}

Expr *manage_assignexpr(Expr *lval, Expr *expr)
{
    Expr *assignexpr = (Expr *)0;
    if (lval && expr)
    {
        if (lval->type == tableitem_e)
        {
            /* use result operand for the assigned value */
            Quads_emit(tablesetelem, expr, lval, lval->index, yylineno);
            assignexpr = emit_iftableitem(lval);
            assignexpr->type = assignexpr_e;
        }
        else
        {
            //lval->sym->value.varVal->val = expr;
            Quads_emit(assign, lval, expr, NULL, yylineno);
            /* we need a new temporary variable because l-value may change */
            assignexpr = Expr_new(assignexpr_e, Quads_newTemp(symtable, scope));
            Quads_emit(assign, assignexpr, lval, NULL, yylineno);
        }
    }
    return assignexpr;
}

Expr *manage_elist_comma_expr(Expr *elist, Expr *expr)
{
    expr->next = elist;
    elist->prev = expr;
    return expr;
}

Expr *manage_primary_funcdef(SymTableEntry *funcdef)
{
    return Expr_new(programfunc_e, funcdef);
}

Expr *manage_primary_lvalue(Expr *lval)
{
    return emit_iftableitem(lval);
}

Expr *manage_table_lvalue_dot_id(Expr *lval, char *id)
{
    //Expr *e_ret = member_item(lval, Expr_strConst(&id));
    return member_item(lval, Expr_strConst(&id));
}

Expr *manage_table_lvalue_index_expr(Expr *lval, Expr *e_index)
{
    return member_item(lval, e_index);
}

Expr *manage_objectdef_elist(Expr *elist)
{
    int index = 0;
    Expr *newTable = Expr_new(newtable_e, Quads_newTemp(symtable, scope));
    Quads_emit(tablecreate, newTable, NULL, NULL, yylineno);

    if (elist != NULL)
    {
        Expr *currExpr = elist;
        /* find the last (first declared) in reversed list */
        while (currExpr->next)
            currExpr = currExpr->next;
        /* cross the list from the first declared param to last (last -> head inside the list using prev ptr) */
        while (currExpr)
        {
            Quads_emit(tablesetelem, currExpr, newTable, Expr_numConst(index++), yylineno);
            currExpr = currExpr->prev;
        }
    }

    return newTable;
}

Expr *manage_objectdef_indexed(HashExprObject *indexed)
{
    HashExprObject *curr = indexed;
    Expr *newTable = Expr_new(newtable_e, Quads_newTemp(symtable, scope));
    Quads_emit(tablecreate, newTable, NULL, NULL, yylineno);

    while (curr)
    {
        Quads_emit(tablesetelem, curr->value, newTable, curr->key, yylineno);
        curr = curr->next;
    }

    HashExprObject_free(indexed);
    indexed = NULL;

    return newTable;
}

F_call manage_methodcall(F_call mcall, char *id, Expr *elist)
{
    mcall.method = 1;
    mcall.elist = elist;
    mcall.name = id;
    return mcall;
}

F_call manage_normcall(F_call ncall, Expr *elist)
{
    ncall.method = 0;
    ncall.elist = elist;
    ncall.name = NULL;
    return ncall;
}

Expr *manage_call_call_elist(Expr *cl, Expr *elist)
{
    return make_call(cl, elist);
}

Expr *manage_call_lvalue_callsuffix(Expr *lval, F_call callsuffix)
{
    if (lval)
    {
        lval = emit_iftableitem(lval);
        if (callsuffix.method)
        {
            Expr *temp = lval;
            Expr *member = member_item(temp, Expr_strConst(&callsuffix.name));
            lval = emit_iftableitem(member);

            if (callsuffix.elist != NULL)
            {
                Expr *currExpr = callsuffix.elist;
                /* cross the expression list fromt last to first (written) param */
                while (currExpr->next)
                    currExpr = currExpr->next;
                /* insert as first argument (reversed, so last) */
                currExpr->next = temp;
            }
            else
                callsuffix.elist = temp;
        }
        return make_call(lval, callsuffix.elist);
    }
    return NULL;
}

Expr *manage_call_funcdef_elist(SymTableEntry *funcdef, Expr *elist)
{
    Expr *func = Expr_new(programfunc_e, funcdef);
    return make_call(func, elist);
}

Expr *manage_lvalue_id(char *id)
{
    SymTableEntry *symLookUpEntry, *newSymEntry, *openFunc, *exprSym, *libFuncEntry;
    SymbolType type;
    int i, found = 0;

    if (!(libFuncEntry = check_libfunc(id)))
    {
        /* lookup for last open function */
        openFunc = FuncStack_top(&currFunc_Stack);

        /* checks from current scope to global */
        for (i = scope; i > -1; i--)
        {
            /* returns the last symbol appeared inside the specific scope */
            symLookUpEntry = ScopeList_lastlookup(symtable->scopeList, id, i);

            /* if found and it is active */
            if (symLookUpEntry != NULL && symLookUpEntry->isActive)
            {
                found = 1;

                /* check if mediates a function between declaration and reference of the current symbol */
                if (openFunc && openFunc->scope != 0 && symLookUpEntry->scope != 0 && openFunc->scope >= symLookUpEntry->scope && symLookUpEntry->type != USERFUNC)
                {
                    COMPILE_ERROR(stderr, "Illegal access of symbol \"%s\" inside function \"%s\" (located in %d)!", id, openFunc->name, openFunc->line)
                    exprSym = symLookUpEntry;
                }
                else /* else, refer in it */
                {
                    if (symLookUpEntry->type == USERFUNC)
                        PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to function \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                    else if (symLookUpEntry->type == LOCAL)
                        PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to local var \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                    else if (symLookUpEntry->type == GLOBAL)
                        PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to global var \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                    else
                        PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to formal argument \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                }

                exprSym = symLookUpEntry;

                break;
            }
        }

        if (!found)
        {
            if (scope == 0)
            {
                PRINT_OK(stderr, "OK => insert global var \"%s\" located in line %d, to ST", id, yylineno)
                type = GLOBAL;
            }
            else
            {
                PRINT_OK(stderr, "OK => insert local var \"%s\" located in line %d, to ST", id, yylineno)
                type = LOCAL;
            }

            newSymEntry = SymTable_insert(symtable, id, scope, yylineno, true, Variable_new(), Function_new(), type);
            update_scopeSpace_offset(newSymEntry);
            exprSym = newSymEntry;
        }
    }
    else
    {
        PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to library function \"%s\"", id, yylineno, id)
        exprSym = libFuncEntry;
    }

    free(id);
    return Expr_lvalue(exprSym);
}

Expr *manage_lvalue_local_id(char *id)
{
    SymTableEntry *newSymEntry, *exprSym, *symLookUpEntry = ScopeList_lookup(symtable->scopeList, id, scope);
    /* check if the given symbol exists in the same scope */
    if (symLookUpEntry == NULL) /* does not exist */
    {
        /* check for collision with lib function when the decleration located to non-global scope */
        if (check_libfunc(id) && scope != 0)
        {
            COMPILE_ERROR(stderr, "Collision with library function \"%s\"!", id)
            exprSym = symLookUpEntry;
        }
        else
        {
            SymbolType type;
            if (scope == 0)
            {
                type = GLOBAL;
                PRINT_OK(stderr, "OK => insert global var \"%s\" located in line %d, to ST", id, yylineno)
            }
            else
            {
                type = LOCAL;
                PRINT_OK(stderr, "OK => insert local var \"%s\" located in line %d, to ST", id, yylineno)
            }

            newSymEntry = SymTable_insert(symtable, id, scope, yylineno, true, Variable_new(), Function_new(), type);
            update_scopeSpace_offset(newSymEntry);
            exprSym = newSymEntry;
        }
    }
    else /* exists */
    {
        /* if there is another variable with the same id on another block but deactivated */
        if (symLookUpEntry->isActive == 0)
        {
            SymbolType type;
            if (scope == 0)
            {
                type = GLOBAL;
                PRINT_OK(stderr, "OK => insert global var \"%s\" located in line %d, to ST", id, yylineno)
            }
            else
            {
                type = LOCAL;
                PRINT_OK(stderr, "OK => insert local var \"%s\" located in line %d, to ST", id, yylineno)
            }

            newSymEntry = SymTable_insert(symtable, id, scope, yylineno, true, Variable_new(), Function_new(), LOCAL);
            update_scopeSpace_offset(newSymEntry);
            exprSym = newSymEntry;
        }
        else
        {
            if (symLookUpEntry->type == LIBFUNC && scope != 0)
            {
                COMPILE_ERROR(stderr, "Collision with library function \"%s\"!", symLookUpEntry->name)
            }
            else
            {
                if (symLookUpEntry->type == LIBFUNC)
                    PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d (global scope) -> refers to library function \"%s\"", id, yylineno, symLookUpEntry->name)
                else if (symLookUpEntry->type == USERFUNC)
                    PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to user function \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                else if (symLookUpEntry->type == FORMAL)
                    PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to formal argument \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                else if (symLookUpEntry->type == LOCAL)
                    PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to local var \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
                else
                    PRINT_OK(stderr, "OK => lookup symbol \"%s\" located in line %d -> refers to global var \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
            }

            exprSym = symLookUpEntry;
        }
    }
    free(id);
    if (exprSym)
        return Expr_lvalue(exprSym);
    return NULL;
}

Expr *manage_lvalue_global_id(char *id)
{
    SymTableEntry *symLookUpEntry = ScopeList_lookup(symtable->scopeList, id, 0);
    /* lookup in global scope for the specific id */
    if (symLookUpEntry == NULL)
    {
        COMPILE_ERROR(stderr, "Symbol \"%s\" not found in global scope!", id)
    }
    else
    {
        if (symLookUpEntry->type == LIBFUNC)
            PRINT_OK(stderr, "OK => lookup symbol \"::%s\" located in line %d -> refers to library function \"%s\"", id, yylineno, symLookUpEntry->name)
        else if (symLookUpEntry->type == USERFUNC)
            PRINT_OK(stderr, "OK => lookup symbol \"::%s\" located in line %d -> refers to global function \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
        else
            PRINT_OK(stderr, "OK => lookup symbol \"::%s\" located in line %d -> refers to global var \"%s\" located in line %d", id, yylineno, symLookUpEntry->name, symLookUpEntry->line)
    }
    free(id);
    if (symLookUpEntry)
        return Expr_lvalue(symLookUpEntry);
    return NULL;
}

SymTableEntry *manage_funcname_id(char *id)
{
    SymTableEntry *symEntry = NULL;
    /* check for collision.. */
    if (check_libfunc(id))
    {
        COMPILE_ERROR(stderr, "Collision with library function \"%s\"!", id)
    }
    else
    {
        SymTableEntry *symLookUpEntry = ScopeList_lookup(symtable->scopeList, id, scope);
        /* check if its active in the same scope or not */
        if (symLookUpEntry != NULL && symLookUpEntry->isActive) /* found */
        {
            COMPILE_ERROR(stderr, "Symbol \"%s\" already defined (in line %d)!", id, symLookUpEntry->line)
        }
        else /* not found */
        {
            /* count only the valid functions */
            PRINT_OK(stderr, "OK => insert function \"%s\" located in line %d, to ST", id, yylineno)
            symEntry = SymTable_insert(symtable, id, scope, yylineno, true, Variable_new(), Function_new(), USERFUNC);
        }
    }

    free(id);
    return symEntry;
}

SymTableEntry *manage_funcname_anonymous()
{
    SymTableEntry *symEntry;
    char *func_name = createNewSymbolName(anonymFuncCounter++, 1);
    PRINT_OK(stderr, "OK => insert anonymous function \"%s\" located in line %d, to ST", func_name, yylineno)
    symEntry = SymTable_insert(symtable, func_name, scope, yylineno, true, Variable_new(), Function_new(), USERFUNC);
    free(func_name);
    return symEntry;
}

SymTableEntry *manage_funcdef_funcprefix(SymTableEntry *funcprefix)
{
    if (funcprefix)
    {
        FuncStack_push(&currFunc_Stack, funcprefix);

        Quads_emit(jump, NULL, NULL, NULL, yylineno);
        OffsetStack_push(&jumpFunc_Stack, Quads_nextQuad() - 1);
        funcprefix->value.funcVal->icode_addr = Quads_nextQuad();
        Quads_emit(funcstart, Expr_new(programfunc_e, funcprefix), NULL, NULL, yylineno);
        OffsetStack_push(&offset_Stack, currScopeOffset());

        /* entering formal arguments scope space */
        enterScopeSpace();
        resetFormalArgsOffset();
    }
    return funcprefix;
}

void manage_funcbodystart()
{
    OffsetStack_push(&loopCounter_Stack, loopCounter);
    loopCounter = 0;
    insideFuncCounter++;
}

unsigned int manage_funcbodyend()
{
    unsigned int total_locals = currScopeOffset();
    exitScopeSpace();
    loopCounter = OffsetStack_pop(&loopCounter_Stack);
    insideFuncCounter--;
    return total_locals;
}

SymTableEntry *manage_funcdef_funcbody(SymTableEntry *sym, unsigned int total_args, unsigned int total_locals)
{
    if (sym)
    {
        SymTableEntry *currFuncEntry;

        exitScopeSpace();
        sym->value.funcVal->total_locals = total_locals;
        sym->value.funcVal->total_args = total_args;

        restoreCurrScopeOffset(OffsetStack_pop(&offset_Stack));
        Quads_emit(funcend, Expr_new(programfunc_e, sym), NULL, NULL, yylineno);
        Quads_patchLabel(OffsetStack_pop(&jumpFunc_Stack), Quads_nextQuad());

        currFuncEntry = FuncStack_pop(&currFunc_Stack);

        QuadList_backpatch(currFuncEntry->value.funcVal->returnList, Quads_nextQuad() - 1);
        //QuadList_free(&currFuncEntry->value.funcVal->returnList);
        currFuncEntry->value.funcVal->returnList = NULL;
    }
    return sym;
}

void manage_idlist_id(char *id)
{
    SymTableEntry *newEntry = NULL;
    /* check for collision.. */
    if (check_libfunc(id))
    {
        COMPILE_ERROR(stderr, "Collision with library function \"%s\"!", id)
    }
    else
    {
        SymTableEntry *symEntry = ScopeList_lastlookup(symtable->scopeList, id, scope + 1);
        if (symEntry != NULL && symEntry->isActive)
        {
            COMPILE_ERROR(stderr, "Formal argument \"%s\" already defined!", id)
        }
        else
        {
            PRINT_OK(stderr, "OK => insert formal argument \"%s\" located in line %d, to ST", id, yylineno)
            newEntry = SymTable_insert(symtable, id, scope + 1, yylineno, true, Variable_new(), Function_new(), FORMAL);
            update_scopeSpace_offset(newEntry);
        }
    }

    free(id);
}

stmt manage_returnstmt(stmt returnstmt, Expr *expr)
{
    if (!insideFuncCounter)
    {
        COMPILE_ERROR(stderr, "Use of \"return\" statement while not in a function!")
    }
    else
    {
        if (expr && FuncStack_size(&currFunc_Stack))
        {
            Quads_emit(ret, expr, NULL, NULL, yylineno);
            Quads_emit(jump, NULL, NULL, NULL, yylineno);

            /* insert return to the quadlist inside function's symbol table entry */
            QuadList *newNode = QuadList_new(Quads_nextQuad() - 1);
            QuadList *curr = FuncStack_top(&currFunc_Stack)->value.funcVal->returnList;
            if (curr)
            {
                while (curr->next)
                    curr = curr->next;
                curr->next = newNode;
            }
            else
                FuncStack_top(&currFunc_Stack)->value.funcVal->returnList = newNode;
        }
    }
    Stmt_makeList(&returnstmt);
    return returnstmt;
}

#endif