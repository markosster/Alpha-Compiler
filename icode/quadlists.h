#ifndef QUADLISTS_H
#define QUADLISTS_H

#include <stdio.h>

typedef struct stmt
{
    /*unsigned int breakList;
    unsigned int contList;*/
    struct QuadList *breakList;
    struct QuadList *contList;
} stmt;

typedef struct QuadList
{
    unsigned int quad;
    struct QuadList *next;
    struct QuadList *nextInFreeList;
} QuadList;

typedef struct forprefix_s
{
    unsigned int test;
    unsigned int enter;
} forprefix_s;

extern QuadList *qlFreeList_head;
extern QuadList *qlFreeList_last;
void QlFreeList_insert(QuadList *ql);
void QlFreeList_free(void);

void Stmt_makeList(stmt *s);
/*int Stmt_newList(int i);
int Stmt_mergeList(int l1, int l2);
void Stmt_patchList(int list, int label);*/

QuadList *QuadList_new(unsigned int quad);
QuadList *QuadList_merge(QuadList *l1, QuadList *l2);
void QuadList_backpatch(QuadList *list, unsigned int quad);
void QuadList_display(FILE *fout, QuadList *list);
void QuadList_free(QuadList **list);

#endif