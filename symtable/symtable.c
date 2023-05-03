#include "symtable.h"

/* calculates the index for the specific symbol inside symbol table (HashTable) */
static unsigned int SymTable_hash(const char *key)
{
    size_t ui;
    unsigned int uiHash = 0U;
    for (ui = 0U; key[ui] != '\0'; ui++)
        uiHash = uiHash * HASH_MULTIPLIER + key[ui];
    return (uiHash % BUCKETS);
}

static const char *symType_toString(SymbolType type)
{
    switch (type)
    {
    case GLOBAL:
        return "global variable";
    case LOCAL:
        return "local variable";
    case FORMAL:
        return "formal argument";
    case USERFUNC:
        return "user function";
    case LIBFUNC:
        return "library function";
    default:
        return NULL;
    }
}

/* initialize library function inside the symbol table */
static void SymTable_addLibFunctions(SymTable *symtable)
{
    SymTable_insert(symtable, "print", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "input", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "objectmemberkeys", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "objecttotalmembers", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "objectcopy", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "totalarguments", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "argument", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "typeof", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "strtonum", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "sqrt", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "cos", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
    SymTable_insert(symtable, "sin", 0, 0, true, Variable_new(), Function_new(), LIBFUNC);
}

SymTable *SymTable_init(void)
{
    unsigned int i;
    SymTable *newTable = (SymTable *)malloc(sizeof(struct SymTable));
    assert(newTable);

    for (i = 0; i < BUCKETS; i++)
        newTable->table[i] = (SymTableEntry *)NULL;

    newTable->size = 0;

    /* initialize the scope list for the symbol table
     every entry in this list is a individual list for every scope */
    newTable->scopeList = (ScopeList *)malloc(sizeof(struct ScopeList));
    assert(newTable->scopeList);

    /* initialize head and last pointer to scoplist (of scopelists) */
    newTable->scopeList->headScopeListEntry = (ScopeListEntry *)NULL;
    newTable->scopeList->lastScopeListEntry = (ScopeListEntry *)NULL;

    SymTable_addLibFunctions(newTable);

    return newTable;
}

void SymTable_free(SymTable *symtab)
{
    SymTableEntry *cross = (SymTableEntry *)NULL;
    SymTableEntry *next = (SymTableEntry *)NULL;
    ScopeListEntry *currScopeListEntry = (ScopeListEntry *)NULL;
    unsigned int i;

    assert(symtab);

    /* symbol table */
    for (i = 0; i < BUCKETS; i++)
    {
        cross = symtab->table[i];
        while (cross)
        {
            next = cross->nextInCoList;
            if (cross->name)
                free((char *)cross->name);
            if (cross->value.varVal)
                free(cross->value.varVal);
            if (cross->value.funcVal)
            {
                if (cross->value.funcVal->returnList)
                {
                    QuadList_free(&cross->value.funcVal->returnList);
                    cross->value.funcVal->returnList = NULL;
                }
                free(cross->value.funcVal);
            }
            free(cross);
            cross = next;
        }
    }

    /* scopelist */
    currScopeListEntry = symtab->scopeList->headScopeListEntry;

    while (currScopeListEntry)
    {
        ScopeListEntry *tmpNext = currScopeListEntry->next;
        free(currScopeListEntry);
        currScopeListEntry = tmpNext;
    }

    free(symtab->scopeList);
    free(symtab);
    symtab = (SymTable *)NULL;
}

ScopeListEntry *ScopeList_get(ScopeList *scopelist, unsigned int scope)
{
    ScopeListEntry *currScopeList = NULL;

    assert(scopelist && scope >= 0);

    currScopeList = scopelist->headScopeListEntry;

    while (currScopeList != NULL && currScopeList->scopeNum != scope)
        currScopeList = currScopeList->next;

    if (currScopeList == NULL)
        return NULL; /* scope list doesnt found */

    return currScopeList;
}

SymTableEntry *ScopeList_lookup(ScopeList *scopelist, const char *name, unsigned int scope)
{
    ScopeListEntry *currScopeList = NULL;
    SymTableEntry *currEntry = NULL;

    assert(scopelist && name && scope >= 0);

    currScopeList = ScopeList_get(scopelist, scope);

    if (currScopeList == NULL)
        return NULL; /* scope list doesnt found */

    currEntry = currScopeList->headScopeSymEntry;

    while (currEntry && strcmp(currEntry->name, name))
        currEntry = currEntry->nextInScopeList;

    return currEntry;
}

SymTableEntry *ScopeList_lastlookup(ScopeList *scopelist, const char *name, unsigned int scope)
{
    ScopeListEntry *currScopeList = NULL;
    SymTableEntry *currEntry = NULL, *lastdup = NULL;

    assert(scopelist && name && scope >= 0);

    currScopeList = ScopeList_get(scopelist, scope);

    if (currScopeList == NULL)
        return NULL; /* scope list doesnt found */

    currEntry = currScopeList->headScopeSymEntry;

    while (currEntry != NULL)
    {
        if (strcmp(currEntry->name, name) == 0)
            lastdup = currEntry;
        currEntry = currEntry->nextInScopeList;
    }

    return lastdup;
}

int ScopeList_insert(ScopeList *scopelist, SymTableEntry *symtabEntry, unsigned int scope)
{
    ScopeListEntry *currScopeList;

    assert(scopelist && symtabEntry && scope >= 0);

    /* symtable's scopelist (of scope lists) (initialize header pointer) */
    currScopeList = scopelist->headScopeListEntry;

    /* first entry in the list */
    if (currScopeList == NULL)
    {
        ScopeListEntry *newEntry = (ScopeListEntry *)malloc(sizeof(struct ScopeListEntry));
        assert(newEntry);
        newEntry->headScopeSymEntry = symtabEntry;
        newEntry->lastScopeSymEntry = symtabEntry;
        newEntry->scopeNum = scope;

        newEntry->next = NULL;
        scopelist->headScopeListEntry = newEntry;
        scopelist->lastScopeListEntry = newEntry;
    }
    else
    {
        /* find the entry (scope) in the linked list */
        while (currScopeList && currScopeList->scopeNum != scope)
            currScopeList = currScopeList->next;

        /* not found, create a new entry for this scope */
        if (currScopeList == NULL)
        {
            ScopeListEntry *newEntry = (ScopeListEntry *)malloc(sizeof(struct ScopeListEntry));
            ScopeListEntry *tmp = scopelist->headScopeListEntry;

            assert(newEntry);
            newEntry->headScopeSymEntry = symtabEntry;
            newEntry->lastScopeSymEntry = symtabEntry;
            newEntry->scopeNum = scope;

            /* SORT insert */
            while (tmp->next != NULL && tmp->next->scopeNum < newEntry->scopeNum)
                tmp = tmp->next;

            newEntry->next = tmp->next;
            tmp->next = newEntry;

            /* update last pointer */
            tmp = scopelist->headScopeListEntry;
            while (tmp->next != NULL)
                tmp = tmp->next;

            scopelist->lastScopeListEntry = tmp;
        }
        else
        {
            /* found the scope entry in the scope list,
            change the last symbol in the same scope list to link to the new symbol entry */
            currScopeList->lastScopeSymEntry->nextInScopeList = symtabEntry;
            currScopeList->lastScopeSymEntry = symtabEntry;
        }
    }

    return 1;
}

SymTableEntry *SymTable_insert(SymTable *symtab, const char *name, unsigned int scope, unsigned int line, int isActive, Variable *varVal, Function *funcVal, SymbolType type)
{
    SymTableEntry check;
    unsigned int index;

    assert(symtab && name && scope >= 0);

    index = SymTable_hash(name);

    /*check = ScopeList_lookup(symtab->scopeList, name, scope);
    if (check == NULL || (check != NULL && check->isActive == 0))
    {*/
    SymTableEntry *currSym = symtab->table[index];

    /* then push to a linked list */
    SymTableEntry *newSymEntry = (SymTableEntry *)malloc(sizeof(struct SymTableEntry));
    assert(newSymEntry);

    newSymEntry->name = strdup(name);
    newSymEntry->scope = scope;
    newSymEntry->line = line;
    newSymEntry->isActive = isActive;
    newSymEntry->value.varVal = varVal;
    newSymEntry->value.funcVal = funcVal;
    newSymEntry->type = type;

    newSymEntry->scopeSpace = none;
    newSymEntry->offset = 0;

    /* be the first symbol in the bucket */
    if (currSym == NULL)
        symtab->table[index] = newSymEntry;
    else
    {
        /* find the last symbol in the linked list to add the new one */
        while (currSym->nextInCoList)
            currSym = currSym->nextInCoList;

        currSym->nextInCoList = newSymEntry;
    }
    newSymEntry->nextInCoList = NULL;
    newSymEntry->nextInScopeList = NULL;

    symtab->size++;

    ScopeList_insert(symtab->scopeList, newSymEntry, scope);
    /*}
    else
        return 0;*/

    return newSymEntry;
}

int ScopeList_hide(ScopeList *scopelist, unsigned int scope)
{
    ScopeListEntry *currScopeList = NULL;
    SymTableEntry *currEntry = NULL;

    assert(scopelist && scope >= 0);

    currScopeList = scopelist->headScopeListEntry;

    /* find the right scope linked list */
    while (currScopeList && currScopeList->scopeNum != scope)
        currScopeList = currScopeList->next;

    if (currScopeList == NULL)
        return 0; /* scope list not found */

    currEntry = currScopeList->headScopeSymEntry;

    while (currEntry)
    {
        currEntry->isActive = false;
        currEntry = currEntry->nextInScopeList;
    }

    return 1;
}

void SymTable_display(FILE *fout, SymTable *symtab)
{
    int i, j;

    assert(symtab);

    for (i = 0; i < BUCKETS; i++)
    {
        SymTableEntry *currSymEntry = symtab->table[i];
        if (currSymEntry)
        {
            j = 0;

            PRINT_CYAN(fout, "Bucket#")
            PRINT_BLUE(fout, "%d ", i)

            while (currSymEntry)
            {
                if (++j > 1)
                {
                    int k, c = count_digits(i);
                    for (k = 0; k < c + 8; k++)
                        fprintf(fout, " ");
                }

                fprintf(fout, "-> { [");
                PRINT_PURPLE(fout, "%s", symType_toString(currSymEntry->type))
                fprintf(fout, "] (");
                PRINT_YELLOW(fout, "line %u", currSymEntry->line)
                fprintf(fout, ") (");
                PRINT_CYAN(fout, "scope %u", currSymEntry->scope)
                fprintf(fout, ") (");
                PRINT_GREEN(fout, "offset %u", currSymEntry->offset)
                fprintf(fout, ") (");
                PRINT_GREEN(fout, "%s", scopeSpace_toString(currSymEntry->scopeSpace))
                fprintf(fout, ") \"");
                PRINT_BLUE(fout, "%s", currSymEntry->name)
                fprintf(fout, "\" } ->\n");
                currSymEntry = currSymEntry->nextInCoList;
            }
        }
    }
}

void ScopeList_display(FILE *fout, SymTable *symtab)
{
    ScopeListEntry *currScope = NULL;

    assert(symtab && symtab->scopeList);

    currScope = symtab->scopeList->headScopeListEntry;
    while (currScope)
    {
        SymTableEntry *currSymEntry = currScope->headScopeSymEntry;

        fprintf(fout, "----------------  ");
        PRINT_CYAN(fout, "Scope #%d", currScope->scopeNum)
        fprintf(fout, " ----------------\n");

        while (currSymEntry)
        {
            fprintf(fout, "[");
            PRINT_PURPLE(fout, "%s", symType_toString(currSymEntry->type))
            fprintf(fout, "] (");
            PRINT_YELLOW(fout, "line %u", currSymEntry->line)
            fprintf(fout, ") (");
            PRINT_CYAN(fout, "scope %d", currSymEntry->scope)
            fprintf(fout, ") \"");
            PRINT_BLUE(fout, "%s", currSymEntry->name)
            fprintf(fout, "\"\n");
            currSymEntry = currSymEntry->nextInScopeList;
        }
        currScope = currScope->next;
    }
}

Function *Function_new()
{
    Function *f = (Function *)malloc(sizeof(struct Function));
    f->returnList = NULL;
    return f;
}

Variable *Variable_new()
{
    return (Variable *)malloc(sizeof(struct Variable));
}