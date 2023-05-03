#include "libfuncs.h"

libfuncs_hashtable_bucket *libfuncs_hashtable[AVM_LIBFUNCS_HASHSIZE];
unsigned libfuncsTotal;

void libfunc_print(void)
{
    unsigned i, n = avm_totalactuals();
    for (i = 0; i < n; i++)
    {
        char *s = avm_tostring(avm_get_actual(i));
        if (s)
        {
            fprintf(stdout, "%s", s);
            free(s);
        }
        /*if (avm_get_actual(i)->type == table_m)
            fprintf(stdout, "\n");*/
    }
    fflush(stdout);
}
void libfunc_input(void)
{
}
void libfunc_objectmemberkeys(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'objectmemberkeys' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *table_arg = avm_get_actual(0);

        if (table_arg->type != table_m)
        {
            AVM_ERROR("objectmemberkeys: Argument not a table!")
            retval.type = nil_m;
        }
        else
        {
            if (table_arg->data.tableVal->total == 0)
                AVM_WARNING("objectmemberkeys: No table elements!")

            avm_table_bucket *currOrd = table_arg->data.tableVal->tblOrdered;
            avm_table *memberKeysTable = avm_table_new();
            unsigned i = 0;

            avm_memcell_clear(&ax);

            while (currOrd)
            {
                ax.type = number_m;
                ax.data.numVal = i;

                avm_table_setelem(memberKeysTable, &ax, currOrd->key);

                currOrd = currOrd->nextInOrder;
                ++i;
            }
            retval.type = table_m;
            retval.data.tableVal = memberKeysTable;
        }
    }
}
void libfunc_objecttotalmembers(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'objecttotalmembers' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *table_arg = avm_get_actual(0);

        if (table_arg->type != table_m)
        {
            AVM_ERROR("objecttotalmembers: Argument not a table!")
            retval.type = nil_m;
        }
        else
        {
            retval.type = number_m;
            retval.data.numVal = table_arg->data.tableVal->total;
        }
    }
}
void libfunc_objectcopy(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'objectcopy' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *table_arg = avm_get_actual(0);

        if (table_arg->type != table_m)
        {
            AVM_ERROR("objectcopy: Argument not a table!")
            retval.type = nil_m;
        }
        else
        {
            avm_memcell_assign(&retval, table_arg);
            retval.type = table_m;
        }
    }
}
void libfunc_totalarguments(void)
{
    /* get topsp of previous activation record */
    unsigned prev_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    avm_memcell_clear(&retval);

    if (!prev_topsp)
    {
        AVM_ERROR("Library function 'totalarguments' called outside a function!")
        retval.type = nil_m;
    }
    else
    {
        /* Extract the number of actual arguments for the previous activation record */
        retval.type = number_m;
        retval.data.numVal = avm_get_envvalue(prev_topsp + AVM_NUMACTUALS_OFFSET);
    }
}
void libfunc_argument(void)
{
    /* get topsp of previous activation record */
    unsigned prev_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    avm_memcell_clear(&retval);

    if (prev_topsp == AVM_STACKSIZE - 1)
    {
        AVM_ERROR("Library function 'argument' called outside a function!")
        retval.type = nil_m;
    }
    else
    {
        unsigned n = avm_totalactuals(); /* activation record: argument */
        unsigned prev_totalactuals = stack[prev_topsp + AVM_NUMACTUALS_OFFSET].data.numVal;

        if (n != 1)
        {
            AVM_ERROR("One argument expected in 'argument' library function!")
            retval.type = nil_m;
        }
        else
        {
            unsigned offset = avm_get_actual(0)->data.numVal;
            // printf("prev_totalactuals: %u, offset: %u\n", prev_totalactuals - 1, offset);
            if (offset + 1 > prev_totalactuals)
            {
                /*AVM_WARNING("Wrong argument offset!")*/
                retval.type = nil_m;
            }
            else
                avm_memcell_assign(&retval, &stack[prev_topsp + AVM_NUMACTUALS_OFFSET + offset + 1]);
        }
    }
}
void libfunc_typeof(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
        AVM_ERROR("One argument expected in 'typeof' library function (%d found)!", n)
    else
    {
        retval.type = string_m;
        retval.data.strVal = strdup(typeStrings[avm_get_actual(0)->type]);
    }
}
void libfunc_strtonum(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'strtonum' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *str = avm_get_actual(0);

        if (str->type != string_m)
        {
            AVM_ERROR("Library function 'strtonum' called with non-string type argument!")
            retval.type = nil_m;
        }
        else
        {
            if (!isStrNum(str->data.strVal))
            {
                AVM_WARNING("String's content not a number!")
                retval.type = nil_m;
            }
            else
            {
                retval.type = number_m;
                retval.data.numVal = atof(str->data.strVal);
            }
        }
    }
}
void libfunc_sqrt(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'sqrt' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *num = avm_get_actual(0);

        if (num->type != number_m)
        {
            AVM_ERROR("Library function 'sqrt' called with non-number type argument!")
            retval.type = nil_m;
        }
        else
        {
            if (num->data.numVal < 0)
            {
                AVM_WARNING("Library function 'sqrt' called with negative number as argument!")
                retval.type = nil_m;
                return;
            }

            retval.type = number_m;
            retval.data.numVal = sqrt(num->data.numVal);
        }
    }
}
void libfunc_cos(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'cos' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *num = avm_get_actual(0);

        if (num->type != number_m)
        {
            AVM_ERROR("Library function 'cos' called with non-number type argument!")
            retval.type = nil_m;
        }
        else
        {
            retval.type = number_m;
            retval.data.numVal = cos(num->data.numVal);
        }
    }
}
void libfunc_sin(void)
{
    unsigned n = avm_totalactuals();
    avm_memcell_clear(&retval);

    if (n != 1)
    {
        AVM_ERROR("One argument expected in 'sin' library function (%d found)!", n)
        retval.type = nil_m;
    }
    else
    {
        avm_memcell *num = avm_get_actual(0);

        if (num->type != number_m)
        {
            AVM_ERROR("Library function 'sin' called with non-number type argument!")
            retval.type = nil_m;
        }
        else
        {
            retval.type = number_m;
            retval.data.numVal = sin(num->data.numVal);
        }
    }
}

void avm_call_libfunc(char *id)
{
    libfuncs_hashtable_bucket *libfunc_hash_bucket = avm_libfuncs_get(id);

    if (!libfunc_hash_bucket)
        AVM_ERROR("Unsupported library function '%s' called!", id)
    else
    {
        library_func_t libfunc = libfunc_hash_bucket->library_func;

        /* Enter function and exit function manually */
        topsp = top; /* Enter function sequence. No stack locals */
        totalActuals = 0;

        if (libfunc)
            (*libfunc)();

        if (!executionFinished) /* An error may occur inside library function */
            execute_FUNCEXIT((Instruction *)0);
    }
}

/* ----------- LIBFUNCS HASHTABLE ----------- */

void avm_libfuncs_register(char *id, library_func_t libfunc)
{
    unsigned index = strhash(id, AVM_LIBFUNCS_HASHSIZE);
    libfuncs_hashtable_bucket *new_libfunc = malloc(sizeof(struct libfuncs_hashtable_bucket));
    new_libfunc->id = strdup(id);
    new_libfunc->library_func = libfunc;
    new_libfunc->next = NULL;

    if (libfuncs_hashtable[index] == NULL)
        libfuncs_hashtable[index] = new_libfunc;
    else
    {
        libfuncs_hashtable_bucket *curr = libfuncs_hashtable[index];

        while (curr->next)
            curr = curr->next;

        curr->next = new_libfunc;
    }
    libfuncsTotal++;
}
libfuncs_hashtable_bucket *avm_libfuncs_get(char *id)
{
    unsigned index = strhash(id, AVM_LIBFUNCS_HASHSIZE);
    assert(index >= 0 && index < AVM_LIBFUNCS_HASHSIZE);

    libfuncs_hashtable_bucket *lf = libfuncs_hashtable[index];

    while (lf)
    {
        if (!strcmp(lf->id, id))
            return lf;
        lf = lf->next;
    }

    return NULL;
}
void avm_libfuncs_init()
{
    unsigned i;
    for (i = 0; i < AVM_LIBFUNCS_HASHSIZE; i++)
        libfuncs_hashtable[i] = NULL;

    libfuncsTotal = 0;

    avm_libfuncs_register("print", libfunc_print);
    avm_libfuncs_register("typeof", libfunc_typeof);
    avm_libfuncs_register("totalarguments", libfunc_totalarguments);
    avm_libfuncs_register("argument", libfunc_argument);
    avm_libfuncs_register("cos", libfunc_cos);
    avm_libfuncs_register("sin", libfunc_sin);
    avm_libfuncs_register("sqrt", libfunc_sqrt);
    avm_libfuncs_register("strtonum", libfunc_strtonum);
    avm_libfuncs_register("objecttotalmembers", libfunc_objecttotalmembers);
    avm_libfuncs_register("objectmemberkeys", libfunc_objectmemberkeys);
    avm_libfuncs_register("objectcopy", libfunc_objectcopy);
}
void avm_libfuncs_display(FILE *fout)
{
    unsigned i;
    fprintf(fout, "Library Functions HashTable");
    for (i = 0; i < AVM_LIBFUNCS_HASHSIZE; i++)
    {
        libfuncs_hashtable_bucket *curr = libfuncs_hashtable[i];

        fprintf(fout, "Bucket[%u] -> ", i);

        while (curr)
        {
            fprintf(fout, "\"%s\" -> ", curr->id);
            curr = curr->next;
        }
        fprintf(fout, "\n");
    }
}
void avm_libfuncs_free()
{
    unsigned i;
    for (i = 0; i < AVM_LIBFUNCS_HASHSIZE; i++)
    {
        libfuncs_hashtable_bucket *tmp, *curr = libfuncs_hashtable[i];

        while (curr)
        {
            tmp = curr->next;
            free(curr->id);
            free(curr);
            curr = tmp;
        }
    }
}