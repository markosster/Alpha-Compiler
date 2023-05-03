#include "offsetstack.h"

void OffsetStack_free(OffsetStack **OffsetStackTop)
{
    OffsetStack *tmp, *cross = *OffsetStackTop;
    while (cross)
    {
        tmp = cross->next;
        free(cross);
        cross = tmp;
    }
}

int OffsetStack_size(OffsetStack **OffsetStackTop)
{
    OffsetStack *cross = *OffsetStackTop;
    int size = 0;
    while (cross)
    {
        size++;
        cross = cross->next;
    }
    return size;
}

void OffsetStack_push(OffsetStack **OffsetStackTop, unsigned int value)
{
    OffsetStack *newOffsetStackEntry = (OffsetStack *)malloc(sizeof(struct OffsetStack));
    newOffsetStackEntry->value = value;
    
    newOffsetStackEntry->next = *OffsetStackTop;
    *OffsetStackTop = newOffsetStackEntry;
}

unsigned int OffsetStack_pop(OffsetStack **OffsetStackTop)
{
    OffsetStack *tmp;
    int ret = -1;

    if ((*OffsetStackTop) != NULL)
    {
        ret = (*OffsetStackTop)->value;
        tmp = (*OffsetStackTop)->next;
        free(*OffsetStackTop);
        *OffsetStackTop = tmp;
    }

    return ret;
}

unsigned int OffsetStack_top(OffsetStack **OffsetStackTop)
{
    if (*OffsetStackTop)
        return (*OffsetStackTop)->value;
    return -1;
}

void OffsetStack_display(FILE *fout, OffsetStack **OffsetStackTop)
{
    OffsetStack *cross = *OffsetStackTop;
    int i = 0;

    fprintf(fout, "OffsetStack\n");
    fprintf(fout, "-------------\n");
    if (!cross)
        fprintf(fout, " EMPTY Stack \n");
    while (cross)
    {
        if (++i == 1)
            fprintf(fout, "TOP -> ");
        else
            fprintf(fout, "       ");

        fprintf(fout, "[ %d ]\n", cross->value);
        cross = cross->next;
    }
    fprintf(fout, "-------------\n");
}