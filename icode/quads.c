#include "quads.h"

/* dynamic array for quads */
Quad *quads = (Quad *)0;
unsigned int totalQuads = 0;
unsigned int currQuad = 0;
unsigned int tempCounter = 0;

static const char *iopcode_toString(iopcode opcode)
{
    switch (opcode)
    {
    case jump:
        return "JUMP";
    case assign:
        return "ASSIGN";
    case add:
        return "ADD";
    case sub:
        return "SUB";
    case mul:
        return "MUL";
    case div_op:
        return "DIV";
    case mod:
        return "MOD";
    case uminus:
        return "UMINUS";
    case and:
        return "AND";
    case or:
        return "OR";
    case not:
        return "NOT";
    case if_eq:
        return "IF_EQ";
    case if_noteq:
        return "IF_NOTEQ";
    case if_lesseq:
        return "IF_LESSEQ";
    case if_greatereq:
        return "IF_GREATEREQ";
    case if_less:
        return "IF_LESS";
    case if_greater:
        return "IF_GREATER";
    case call:
        return "CALL";
    case param:
        return "PARAM";
    case ret:
        return "RETURN";
    case getretval:
        return "GETRETVAL";
    case funcstart:
        return "FUNCSTART";
    case funcend:
        return "FUNCEND";
    case tablecreate:
        return "TABLECREATE";
    case tablegetelem:
        return "TABLEGETELEM";
    case tablesetelem:
        return "TABLESETELEM";
    default:
        return "NONE";
    }
}

static void print_dashes(FILE *fout, int dash_width)
{
    int j;
    for (j = 0; j < dash_width; j++)
        fprintf(fout, "%c", '-');
    fprintf(fout, "\n");
}

static void print_blank(FILE *fout, int width)
{
    fprintf(fout, "%-*s ", width, "");
}

static void print_expr_okeanos(FILE *fout, Expr *e)
{
    if (e != NULL)
    {
        if (e->type == var_e ||
            e->type == programfunc_e ||
            e->type == libraryfunc_e ||
            e->type == assignexpr_e ||
            e->type == arithepxr_e ||
            e->type == newtable_e ||
            e->type == tableitem_e ||
            e->type == boolexpr_e)
        {
            if (e->sym)
            {
                if (Quads_isTempVar(e))
                {
                    fprintf(fout, "^");
                    fprintf(fout, "%s ", e->sym->name + 2);
                }
                else if (Quads_isAnonymFunc(e))
                {
                    fprintf(fout, "$");
                    fprintf(fout, "%s ", e->sym->name + 2);
                }
                else
                    fprintf(fout, "%s ", e->sym->name);
            }
            else
                fprintf(fout, "??? ");
        }
        else if (e->type == constnum_e)
        {
            char *fstr = float_tostr(e->numConst);
            fprintf(fout, "%s ", fstr);
            free(fstr);
            //fprintf(fout, "%g ", e->numConst);
        }
        else if (e->type == constbool_e)
        {
            if (e->boolConst == 0)
                fprintf(fout, "'false' ");
            else
                fprintf(fout, "'true' ");
        }
        else if (e->type == conststring_e)
        {
            if (e->strConst)
                fprintf(fout, "\"%s\" ", e->strConst);
            else
                fprintf(fout, "??? ");
        }
        else if (e->type == nil_e)
        {
            fprintf(fout, "nil ");
        }
    }
}

static void print_expr(FILE *fout, Expr *e, int args_width)
{
    if (e != NULL)
    {
        if (e->type == var_e ||
            e->type == programfunc_e ||
            e->type == libraryfunc_e ||
            e->type == assignexpr_e ||
            e->type == arithepxr_e ||
            e->type == newtable_e ||
            e->type == tableitem_e ||
            e->type == boolexpr_e)
        {
            if (e->sym)
                fprintf(fout, "%-*s ", args_width, e->sym->name);
            else
                fprintf(fout, "%-*s ", args_width, "???");
        }
        else if (e->type == constnum_e)
        {
            char *fstr = float_tostr(e->numConst);
            fprintf(fout, "%-*s ", args_width, fstr);
            free(fstr);
            //fprintf(fout, "%-*g ", args_width, e->numConst);
        }
        else if (e->type == constbool_e)
        {
            if (e->boolConst == 0)
                fprintf(fout, "%-*s ", args_width, "'FALSE'");
            else
                fprintf(fout, "%-*s ", args_width, "'TRUE'");
        }
        else if (e->type == conststring_e)
        {
            if (e->strConst)
            {
                fprintf(fout, "\"%s", e->strConst);
                fprintf(fout, "%-*s ", (int)(args_width - strlen(e->strConst) - 1), "\"");
            }
            else
                fprintf(fout, "%-*s ", args_width, "???");
        }
        else if (e->type == nil_e)
        {
            fprintf(fout, "%-*s ", args_width, "'NIL'");
        }
    }
    else
        print_blank(fout, args_width);
}

int check_if_jump_command(iopcode opcode)
{
    if (opcode == if_eq ||
        opcode == if_greater ||
        opcode == if_greatereq ||
        opcode == if_less ||
        opcode == if_lesseq ||
        opcode == if_noteq ||
        opcode == jump)
        return 1;
    return 0;
}

/* ----------------------- QUADS -------------------------- */

void Quads_expand(void)
{
    assert(currQuad == totalQuads);
    Quad *p = (Quad *)malloc(NEW_SIZE);
    if (quads)
    {
        memcpy(p, quads, CURR_SIZE);
        free(quads);
    }
    quads = p;
    totalQuads += EXPAND_SIZE;
}

void Quads_emit(iopcode opcode, Expr *result, Expr *arg1, Expr *arg2, unsigned int line)
{
    if (currQuad == totalQuads)
        Quads_expand();

    quads[currQuad].op = opcode;
    quads[currQuad].result = result;
    quads[currQuad].arg1 = arg1;
    quads[currQuad].arg2 = arg2;
    quads[currQuad].label = 0;
    quads[currQuad].taddress = 0;
    quads[currQuad].line = line;

    currQuad++;
}

unsigned int Quads_nextQuad(void) { return currQuad; }

void Quads_patchLabel(unsigned int quadNo, unsigned int label)
{
    assert(quadNo < currQuad);
    quads[quadNo].label = label;
}

void Quads_okeanos_display(FILE *fout)
{
    unsigned int i;
    char *op;

    for (i = 0; i < currQuad; i++)
    {
        if (quads[i].label != 0 && !check_if_jump_command(quads[i].op))
            fprintf(fout, "UNEXPECTED ERROR @ QUAD#%d=> NON JUMP OP HAS NON ZERO LABEL\n", i + 1);
        else
        {
            op = toLowerString(iopcode_toString(quads[i].op));
            fprintf(fout, "%u: ", i + 1);
            fprintf(fout, "%s ", op);
            free(op);

            if (quads[i].op == tablesetelem)
            {
                print_expr_okeanos(fout, quads[i].arg1);
                print_expr_okeanos(fout, quads[i].arg2);
                print_expr_okeanos(fout, quads[i].result);
            }
            else
            {
                print_expr_okeanos(fout, quads[i].result);
                print_expr_okeanos(fout, quads[i].arg1);
                print_expr_okeanos(fout, quads[i].arg2);
            }

            if (quads[i].label != 0 || check_if_jump_command(quads[i].op))
                fprintf(fout, "%u ", quads[i].label + 1);
            fprintf(fout, "[line %u]\n", quads[i].line);
        }
    }
}

void Quads_display(FILE *fout, int color)
{
    unsigned int i;
    unsigned int label_width = count_digits(totalQuads) + 5;
    int opcode_width = 15;
    int args_width = 13;
    unsigned int dash_width = 2 * label_width + 72;

    /* column names */
    print_dashes(fout, dash_width);
    if (color)
        PRINT_CYAN(fout, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s\n", label_width, "quad#", opcode_width, "opcode", args_width, "result", args_width, "arg1", args_width, "arg2", label_width, "label", label_width, "sourceline#")
    else
        fprintf(fout, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s\n", label_width, "quad#", opcode_width, "opcode", args_width, "result", args_width, "arg1", args_width, "arg2", label_width, "label", label_width, "sourceline#");
    print_dashes(fout, dash_width);

    for (i = 0; i < currQuad; i++)
    {
        if (quads[i].label != 0 && !check_if_jump_command(quads[i].op))
        {
            if (color)
                PRINT_RED(fout, "UNEXPECTED ERROR @ QUAD#%d=> NON JUMP OP HAS NON ZERO LABEL\n", i + 1)
            else
                fprintf(fout, "UNEXPECTED ERROR @ QUAD#%d=> NON JUMP OP HAS NON ZERO LABEL\n", i + 1);
        }
        else
        {
            /* quad#, opcode */
            if (color)
            {
                PRINT_GREEN(fout, "%-*u ", label_width, i + 1);
                PRINT_PURPLE(fout, "%-*s ", opcode_width, iopcode_toString(quads[i].op));
            }
            else
            {
                fprintf(fout, "%-*u ", label_width, i + 1);
                fprintf(fout, "%-*s ", opcode_width, iopcode_toString(quads[i].op));
            }

            /* result, arg1, arg2 */
            if (color)
                fprintf(fout, "\033[0;34m");

            print_expr(fout, quads[i].result, args_width);
            print_expr(fout, quads[i].arg1, args_width);
            print_expr(fout, quads[i].arg2, args_width);

            if (color)
                fprintf(fout, "\033[0m");

            /* label */
            if (quads[i].label == 0 && !check_if_jump_command(quads[i].op))
                print_blank(fout, label_width);
            else
            {
                if (color)
                    PRINT_GREEN(fout, "%-*u ", label_width, quads[i].label + 1)
                else
                    fprintf(fout, "%-*u ", label_width, quads[i].label + 1);
            }

            /* line */
            if (color)
                PRINT_YELLOW(fout, "%-*u\n", label_width, quads[i].line)
            else
                fprintf(fout, "%-*u\n", label_width, quads[i].line);
        }
    }
    print_dashes(fout, dash_width);
}

void Quads_free(void)
{
    if (quads)
    {
        unsigned int i;
        for (i = 0; i < currQuad; i++)
        {
            Expr_free(&quads[i].result);
            Expr_free(&quads[i].arg1);
            Expr_free(&quads[i].arg2);
        }

        QlFreeList_free();
        ExprFreeList_free();

        free(quads);
        quads = (Quad *)0;
    }
}

/* -------------------- TEMP VARIABLES ------------------- */
char *Quads_newTempName() { return createNewSymbolName(tempCounter++, 0); }

void Quads_resetTemp() { tempCounter = 0; }

unsigned int Quads_isTempVar(Expr *e) { return e->sym && (e->sym->name[0] == '_') && (e->sym->name[1] == 't'); }

unsigned int Quads_isAnonymFunc(Expr *e) { return e->sym && (e->sym->name[0] == '_') && (e->sym->name[1] == 'f'); }

SymTableEntry *Quads_newTemp(SymTable *symtable, unsigned int currScope)
{
    char *name = Quads_newTempName();
    SymTableEntry *sym = ScopeList_lookup(symtable->scopeList, name, currScope);

    if (sym == NULL || (sym && !sym->isActive))
    {   
        if (currScope == 0)
            sym = SymTable_insert(symtable, name, currScope, 0, 1, NULL, NULL, GLOBAL);
        else
            sym = SymTable_insert(symtable, name, currScope, 0, 1, NULL, NULL, LOCAL);

        sym->offset = currScopeOffset();
        sym->scopeSpace = currScopeSpace();
        inCurrScopeOffset();
    }

    free(name);
    return sym;
}