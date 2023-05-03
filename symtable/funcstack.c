#include "funcstack.h"

void FuncStack_free(FuncStack **stackTop)
{
    FuncStack *tmp, *cross = *stackTop;
    while (cross)
    {
        tmp = cross->next;
        free(cross);
        cross = tmp;
    }
}

int FuncStack_size(FuncStack **stackTop)
{
    FuncStack *cross = *stackTop;
    int size = 0;
    while (cross)
    {
        size++;
        cross = cross->next;
    }
    return size;
}

void FuncStack_push(FuncStack **stackTop, SymTableEntry *currFuncEntry)
{
    FuncStack *newStackEntry = (FuncStack *)malloc(sizeof(struct FuncStack));
    newStackEntry->currFuncEntry = currFuncEntry;
    newStackEntry->next = *stackTop;
    *stackTop = newStackEntry;
}

SymTableEntry *FuncStack_pop(FuncStack **stackTop)
{
    FuncStack *tmp;
    SymTableEntry *ret = NULL;

    if ((*stackTop) != NULL)
    {
        ret = (*stackTop)->currFuncEntry;
        tmp = (*stackTop)->next;
        free(*stackTop);
        *stackTop = tmp;
    }

    return ret;
}

SymTableEntry *FuncStack_top(FuncStack **stackTop)
{
    if (*stackTop)
        return (*stackTop)->currFuncEntry;
    return NULL;
}

void FuncStack_display(FILE *fout, FuncStack **stackTop)
{
    FuncStack *cross = *stackTop;
    int i = 0;

    fprintf(fout, "Func Stack\n");
    fprintf(fout, "-------------\n");
    if (!cross)
        fprintf(fout, " EMPTY STACK \n");
    while (cross)
    {
        if (++i == 1)
            fprintf(fout, "TOP -> ");
        else
            fprintf(fout, "       ");

        fprintf(fout, "[ %s ]\n", cross->currFuncEntry->name);
        cross = cross->next;
    }
    fprintf(fout, "-------------\n");
}