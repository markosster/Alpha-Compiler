#include "quadlists.h"
#include "quads.h"

QuadList *qlFreeList_head = NULL;
QuadList *qlFreeList_last = NULL;

void QlFreeList_insert(QuadList *ql)
{
    if (qlFreeList_head == NULL)
    {
        qlFreeList_head = ql;
        qlFreeList_last = ql;
    }
    else
    {
        qlFreeList_last->nextInFreeList = ql;
        qlFreeList_last = ql;
    }
}

void QlFreeList_free(void)
{
    QuadList *curr = qlFreeList_head;

    while (curr)
    {
        QuadList *tmp = curr->nextInFreeList;
        free(curr);
        curr = tmp;
    }
}

void Stmt_makeList(stmt *s)
{
    s->breakList = s->contList = 0;
}

QuadList *QuadList_new(unsigned int quad)
{
    QuadList *newList = malloc(sizeof(struct QuadList));
    newList->quad = quad;
    newList->next = NULL;
    newList->nextInFreeList = NULL;

    QlFreeList_insert(newList);

    return newList;
}

QuadList *QuadList_merge(QuadList *l1, QuadList *l2)
{
    if (!l1)
        return l2;
    if (!l2)
        return l1;

    QuadList *cross = l1;

    while (cross->next)
        cross = cross->next;

    cross->next = l2;
    return l1;
}

void QuadList_backpatch(QuadList *list, unsigned int quad)
{
    QuadList *cross = list;
    while (cross)
    {
        quads[cross->quad].label = quad;
        cross = cross->next;
    }
}

void QuadList_free(QuadList **list)
{
    QuadList *cross = *list;
    while (cross)
    {
        QuadList *tmp = cross->next;
        //free(cross);
        cross = tmp;
    }
    //*list = (QuadList *)NULL;
}

void QuadList_display(FILE *fout, QuadList *list)
{
    QuadList *cross = list;
    while (cross)
    {
        fprintf(fout, "quad: %u\n", cross->quad + 1);
        cross = cross->next;
    }
}