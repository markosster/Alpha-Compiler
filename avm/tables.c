#include "tables.h"

int tableSetElemNil = 0;

/* -------------- STATIC FUNCTIONS -------------- */
static unsigned char avm_cmp_values(avm_table_bucket *curr, avm_memcell *index)
{
    if (curr->key->type == string_m)
        return strcmp(curr->key->data.strVal, index->data.strVal);
    else
        return curr->key->data.numVal != index->data.numVal;
}

static void avm_hashtable_delete(avm_table_bucket **p, avm_memcell *index)
{
    unsigned i;

    assert(index->type == number_m || index->type == string_m);
    if (index->type == number_m)
        i = numhash(index->data.numVal, AVM_TABLE_HASHSIZE);
    else
        i = strhash(index->data.strVal, AVM_TABLE_HASHSIZE);

    avm_table_bucket *prev = NULL, *tmp, *curr = p[i];

    while (curr && avm_cmp_values(curr, index))
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr)
    {
        if (prev == NULL)
            p[i] = p[i]->next;
        else
            prev->next = curr->next;

        avm_memcell_clear(curr->key);
        avm_memcell_clear(curr->value);
        free(curr);
    }
}

static void avm_hashtable_numrehash(avm_table *table, unsigned prevKey, avm_memcell *nextKey)
{
    assert(nextKey && nextKey->type == number_m);

    unsigned iprev = numhash(prevKey, AVM_TABLE_HASHSIZE);
    unsigned inext = numhash(nextKey->data.numVal, AVM_TABLE_HASHSIZE);

    avm_table_bucket *prevone = table->numIndexed[iprev];
    avm_table_bucket *nextone = table->numIndexed[inext];
    avm_table_bucket *prev = NULL, *tmp = NULL;

    while (nextone && nextone->key->data.numVal != nextKey->data.numVal)
    {
        prev = nextone;
        nextone = nextone->next;
    }

    if (nextone)
    {
        /* find the previous hash and replace it with the nextone */
        prevone = table->numIndexed[iprev];
        nextone->key->data.numVal--;

        if (prevone == NULL)
            table->numIndexed[iprev] = nextone;
        else
        {
            while (prevone->next)
                prevone = prevone->next;

            prevone->next = nextone;
        }

        /* release from its previous cell in hashtable */
        if (prev == NULL)
            table->numIndexed[inext] = table->numIndexed[inext]->next;
        else
            prev->next = nextone->next;
    }
}

static void avm_print_hashtable(avm_table *table)
{
    unsigned i;
    avm_table_bucket *ord = table->tblOrdered;

    fprintf(stdout, "String Indexes\n");
    for (i = 0; i < AVM_TABLE_HASHSIZE; ++i)
    {
        avm_table_bucket *sb = table->strIndexed[i];

        if (sb == NULL)
            continue;

        fprintf(stdout, "Bucket[%u] -> ", i);
            
        while (sb)
        {
            fprintf(stdout, "[key: ");
            avm_print_memcell_value(stdout, sb->key);
            fprintf(stdout, ", value: ");
            avm_print_memcell_value(stdout, sb->value);
            if (sb->next == NULL)
                fprintf(stdout, "] -> NULL");
            else
                fprintf(stdout, "] -> ");
            sb = sb->next;
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "Num Indexes\n");
    for (i = 0; i < AVM_TABLE_HASHSIZE; ++i)
    {
        avm_table_bucket *nb = table->numIndexed[i];

        if (nb == NULL)
            continue;

        fprintf(stdout, "Bucket[%u] -> ", i);

        while (nb)
        {
            fprintf(stdout, "[key: ");
            avm_print_memcell_value(stdout, nb->key);
            fprintf(stdout, ", value: ");
            avm_print_memcell_value(stdout, nb->value);
            if (nb->next == NULL)
                fprintf(stdout, "] -> NULL");
            else
                fprintf(stdout, "] -> ");
            nb = nb->next;
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "Ordered List\n");

    if (ord == NULL)
        fprintf(stdout, "HEAD -> NULL");
    else
        fprintf(stdout, "HEAD -> ");

    while (ord)
    {
        fprintf(stdout, "[key: ");
        avm_print_memcell_value(stdout, ord->key);
        fprintf(stdout, ", value: ");
        avm_print_memcell_value(stdout, ord->value);
        if (ord->nextInOrder == NULL)
            fprintf(stdout, "] -> NULL");
        else
            fprintf(stdout, "] -> ");
        ord = ord->nextInOrder;
    }
    fprintf(stdout, "\n");

    fprintf(stdout, "Table's refCounter: %u\n", table->refCounter);
}
/* ---------------------------------------------- */

void avm_table_buckets_init(avm_table_bucket **p)
{
    unsigned i;
    for (i = 0; i < AVM_TABLE_HASHSIZE; ++i)
        p[i] = (avm_table_bucket *)0;
}

avm_table *avm_table_new(void)
{
    avm_table *t = malloc(sizeof(struct avm_table));
    AVM_WIPEOUT(*t);

    t->refCounter = 0;
    t->total = 0;
    t->hasKeys = 0;
    t->tblOrdered = NULL;

    avm_table_buckets_init(t->numIndexed);
    avm_table_buckets_init(t->strIndexed);

    return t;
}

void avm_table_buckets_destroy(avm_table_bucket **p)
{
    unsigned i;
    for (i = 0; i < AVM_TABLE_HASHSIZE; ++i)
    {
        avm_table_bucket *b = p[i];
        while (b)
        {
            avm_table_bucket *del = b;
            b = b->next;
            avm_memcell_clear(del->key);
            avm_memcell_clear(del->value);
            free(del);
        }
        p[i] = (avm_table_bucket *)0;
    }
}

void avm_table_destroy(avm_table *table)
{
    avm_table_buckets_destroy(table->numIndexed);
    avm_table_buckets_destroy(table->strIndexed);
    free(table);
}

void avm_table_inc_refcounter(avm_table *table)
{
    ++table->refCounter;
}
void avm_table_dec_refcounter(avm_table *table)
{
    // assert(table->refCounter > 0);
    --table->refCounter;
    /*if (table->refCounter == 0)
        avm_table_destroy(table);*/
}

void avm_table_delelem(avm_table *table, avm_memcell *elem)
{
    /* delete from ordered list */
    avm_table_bucket *prev = NULL, *tmp, *currOrd = table->tblOrdered;

    tableSetElemNil = 1;

    while (currOrd && (elem != currOrd->value))
    {
        prev = currOrd;
        currOrd = currOrd->next;
    }

    if (currOrd)
    {
        if (!table->hasKeys)
        {
            /* num indexed */
            unsigned numkey = (unsigned)currOrd->key->data.numVal;
            avm_table_bucket *crossOrd = currOrd;

            avm_hashtable_delete(table->numIndexed, currOrd->key);

            while (crossOrd)
            {
                if (crossOrd->next)
                    avm_hashtable_numrehash(table, numkey++, crossOrd->next->key);

                crossOrd = crossOrd->next;
            }
        }
        else /* str indexed */
            avm_hashtable_delete(table->strIndexed, currOrd->key);

        if (prev == NULL)
            table->tblOrdered = table->tblOrdered->nextInOrder;
        else
            prev->nextInOrder = currOrd->nextInOrder;

        table->total--;
    }
}

avm_memcell *avm_table_getelem(avm_table *table, avm_memcell *index)
{
    assert(table && index);

    if (index->type == string_m)
    {
        unsigned i = strhash(index->data.strVal, AVM_TABLE_HASHSIZE);
        avm_table_bucket *curr = table->strIndexed[i];

        while (curr && strcmp(curr->key->data.strVal, index->data.strVal))
            curr = curr->next;

        if (curr)
            return curr->value;
    }
    else if (index->type == number_m)
    {
        unsigned i = numhash(index->data.numVal, AVM_TABLE_HASHSIZE);
        avm_table_bucket *curr = table->numIndexed[i];

        while (curr && curr->key->data.numVal != index->data.numVal)
            curr = curr->next;

        if (curr)
            return curr->value;
    }
    /*else if (index->type == bool_m)
            ;
        else if (index->type == table_m)
            ;
        else if (index->type == libfunc_m)
            ;
        else if (index->type == userfunc_m)
            ;*/

    return NULL;
}

void avm_table_setelem(avm_table *table, avm_memcell *index, avm_memcell *rvalue)
{
    assert(table && index && rvalue);
    avm_memcell *elem = avm_table_getelem(table, index);

    if (elem)
    {
        if (rvalue->type == nil_m)
            avm_table_delelem(table, elem);
        else
            avm_memcell_assign(elem, rvalue);
    }
    else
    {
        avm_table_bucket *newElem = (avm_table_bucket *)malloc(sizeof(struct avm_table_bucket));
        newElem->next = NULL;
        newElem->nextInOrder = NULL;

        /* ------------ copy data ------------ */
        newElem->key = (avm_memcell *)malloc(sizeof(struct avm_memcell));
        newElem->value = (avm_memcell *)malloc(sizeof(struct avm_memcell));

        newElem->key->type = index->type;
        newElem->key->data = index->data;

        newElem->value->type = rvalue->type;
        newElem->value->data = rvalue->data;

        if (index->type == string_m)
            newElem->key->data.strVal = strdup(index->data.strVal);

        if (rvalue->type == string_m)
            newElem->value->data.strVal = strdup(rvalue->data.strVal);
        else if (rvalue->type == libfunc_m)
            newElem->value->data.libfuncVal = strdup(rvalue->data.libfuncVal);
        else if (rvalue->type == userfunc_m)
            newElem->value->data.userfuncVal = rvalue->data.userfuncVal;

        /*avm_memcell_assign(newElem->key, index);
        avm_memcell_assign(newElem->value, rvalue);*/
        /* ----------------------------------- */

        if (index->type == string_m)
        {
            unsigned i = strhash(index->data.strVal, AVM_TABLE_HASHSIZE);

            if (table->strIndexed[i] == NULL)
                table->strIndexed[i] = newElem;
            else
            {
                avm_table_bucket *curr = table->strIndexed[i];

                while (curr->next)
                    curr = curr->next;

                curr->next = newElem;
            }
            table->hasKeys = 1;
        }
        else if (index->type == number_m)
        {
            unsigned i = numhash(index->data.numVal, AVM_TABLE_HASHSIZE);

            if (table->numIndexed[i] == NULL)
                table->numIndexed[i] = newElem;
            else
            {
                avm_table_bucket *curr = table->numIndexed[i];

                while (curr->next)
                    curr = curr->next;

                curr->next = newElem;
            }
        }
        /*else if (index->type == bool_m)
            ;
        else if (index->type == table_m)
            ;
        else if (index->type == libfunc_m)
            ;
        else if (index->type == userfunc_m)
            ;*/
        else
            return;

        /* insert into ordered list */
        if (table->tblOrdered == NULL)
            table->tblOrdered = newElem;
        else
        {
            avm_table_bucket *curr = table->tblOrdered;

            while (curr->nextInOrder)
                curr = curr->nextInOrder;

            curr->nextInOrder = newElem;
        }

        ++table->total;
    }
}

avm_memcell *avm_table_functor(avm_memcell *func)
{
    avm_table_bucket *b = func->data.tableVal->strIndexed[strhash("()", AVM_TABLE_HASHSIZE)];

    if (!b)
    {
        AVM_ERROR("Table has no functor '()'!")
        return NULL;
    }
    else
    {
        if (b->value->type != userfunc_m)
        {
            AVM_WARNING("Table has no functor element to call!")
            return NULL;
        }

        /* returns the actual memory cell of the function from the specific table cell */
        return b->value;
    }
}

unsigned bracketTableTabs = 0;
unsigned insideTableItemTabs = 1;

void print_tabs(unsigned tabs)
{
    unsigned i;
    for (i = 0; i < tabs; i++)
        fprintf(stdout, "   ");
}

void avm_table_display(avm_table *table)
{
    avm_table_bucket *elem = table->tblOrdered;
    char *value;
    int ifStrIndexed = 0;

    if (elem == NULL)
    {
        fprintf(stdout, "[ ]");
        return;
    }

    print_tabs(bracketTableTabs);
    fprintf(stdout, "[");

    while (elem)
    {
        fprintf(stdout, "\n");

        if (table->hasKeys)
        {
            print_tabs(insideTableItemTabs);
            fprintf(stdout, "{ ");

            if (elem->key->type == string_m)
                fprintf(stdout, "\"%s\" : ", elem->key->data.strVal);
            else if (elem->key->type == number_m)
                fprintf(stdout, "%d : ", (int)elem->key->data.numVal);
            else if (elem->key->type == bool_m)
                ;
            else if (elem->key->type == table_m)
                ;
            else if (elem->key->type == libfunc_m)
                ;
            else if (elem->key->type == userfunc_m)
                ;

            if (elem->value->type == table_m && elem->value->data.tableVal->total != 0)
            {
                ifStrIndexed = 1;
                fprintf(stdout, "\n");
            }
        }

        if (elem->value->type != table_m)
        {
            if (!table->hasKeys)
                print_tabs(insideTableItemTabs);

            value = avm_tostring(elem->value);
            if (elem->value->type == string_m)
                fprintf(stdout, "\"%s\" ", value);
            else
                fprintf(stdout, "%s ", value);
            free(value);
        }
        else
        {
            if (elem->value->data.tableVal->total != 0)
            {
                if (ifStrIndexed)
                {
                    bracketTableTabs += 2;
                    insideTableItemTabs += 2;
                }
                else
                {
                    bracketTableTabs += 1;
                    insideTableItemTabs += 1;
                }

                avm_table_display(elem->value->data.tableVal);

                if (ifStrIndexed)
                {
                    bracketTableTabs -= 2;
                    insideTableItemTabs -= 2;
                }
                else
                {
                    bracketTableTabs -= 1;
                    insideTableItemTabs -= 1;
                }
            }
            else
                fprintf(stdout, "[ ] ");
        }

        if (table->hasKeys)
        {
            if (elem->value->type == table_m)
            {
                fprintf(stdout, "\n");
                print_tabs(insideTableItemTabs);
            }
            fprintf(stdout, "}");
        }

        if (elem->nextInOrder != NULL)
            fprintf(stdout, ",");

        elem = elem->nextInOrder;
    }

    fprintf(stdout, "\n");
    print_tabs(bracketTableTabs);
    fprintf(stdout, "]");

    //avm_print_hashtable(table);
}