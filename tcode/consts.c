#include "consts.h"

double *numConsts = (double *)NULL;
unsigned int totalNumConsts = 0;
unsigned int currNumConsts = 0;

char **strConsts = (char **)NULL;
unsigned int totalStrConsts = 0;
unsigned int currStrConsts = 0;

char **libFuncs = (char **)NULL;
unsigned int totalLibFuncs = 0;
unsigned int currLibFuncs = 0;

struct UserFunc *userFuncs = (UserFunc *)NULL;
unsigned int totalUserFuncs = 0;
unsigned int currUserFuncs = 0;

unsigned int UserFunc_lookup(UserFunc func)
{
    unsigned int i;
    for (i=0; i<currUserFuncs; i++)
    {
        /* if exists the same function (with the same taddress) then dont add into the table */
        if (!strcmp(func.id, userFuncs[i].id) &&
        userFuncs[i].address == func.address)
            return i;
    }
    return -1;
}

/* ---------------- ADD ---------------- */
unsigned int NumConsts_add(double num)
{
    if (currNumConsts == totalNumConsts)
        NumConsts_expand();

    numConsts[currNumConsts++] = num;
    return currNumConsts - 1;
}
unsigned int StrConsts_add(char *str)
{
    if (currStrConsts == totalStrConsts)
        StrConsts_expand();

    strConsts[currStrConsts++] = strdup(str);
    return currStrConsts - 1;
}
unsigned int LibFuncs_add(char *libId)
{
    if (currLibFuncs == totalLibFuncs)
        LibFuncs_expand();

    libFuncs[currLibFuncs++] = strdup(libId);
    return currLibFuncs - 1;
}
unsigned int UserFuncs_add(UserFunc uf)
{
    if (currUserFuncs == totalUserFuncs)
        UserFuncs_expand();

    unsigned int func_index = UserFunc_lookup(uf);

    /* not found */
    if (func_index == -1)
    {
        UserFunc f;
        f.address = uf.address;
        f.localSize = uf.localSize;
        f.args = uf.args;
        f.id = strdup(uf.id);

        userFuncs[currUserFuncs++] = f;
        return currUserFuncs - 1;
    }
    /* found */
    return func_index;
}
/* ------------------------------------- */

/* -------------- GETTERS -------------- */
double NumConsts_get(unsigned int index)
{
    if (numConsts)
        return numConsts[index];
    return -1;
}
char *StrConsts_get(unsigned int index)
{
    if (strConsts)
        return strConsts[index];
    return NULL;
}
char *LibFuncs_get(unsigned int index)
{
    if (libFuncs)
        return libFuncs[index];
    return NULL;
}
UserFunc *UserFuncs_get(unsigned int index)
{
    if (userFuncs)
        return &(userFuncs[index]);
    return NULL;
}
/* ------------------------------------- */

/* ---------------- EXPAND ---------------- */
void NumConsts_expand(void)
{
    assert(currNumConsts == totalNumConsts);
    double *p = (double *)malloc(NEW_SIZE_NUMCONSTS);
    if (numConsts)
    {
        memcpy(p, numConsts, CURR_SIZE_NUMCONSTS);
        free(numConsts);
    }
    numConsts = p;
    totalNumConsts += EXPAND_SIZE_CONSTS;
}
void StrConsts_expand(void)
{
    assert(currStrConsts == totalStrConsts);
    char **p = (char **)malloc(NEW_SIZE_STRCONSTS);
    if (strConsts)
    {
        memcpy(p, strConsts, CURR_SIZE_STRCONSTS);
        free(strConsts);
    }
    strConsts = p;
    totalStrConsts += EXPAND_SIZE_CONSTS;
}
void LibFuncs_expand(void)
{
    assert(currLibFuncs == totalLibFuncs);
    char **p = (char **)malloc(NEW_SIZE_LIBFUNCS);
    if (libFuncs)
    {
        memcpy(p, libFuncs, CURR_SIZE_LIBFUNCS);
        free(libFuncs);
    }
    libFuncs = p;
    totalLibFuncs += EXPAND_SIZE_CONSTS;
}
void UserFuncs_expand(void)
{
    assert(currUserFuncs == totalUserFuncs);
    struct UserFunc *p = (struct UserFunc *)malloc(NEW_SIZE_USERFUNCS);
    if (userFuncs)
    {
        memcpy(p, userFuncs, CURR_SIZE_USERFUNCS);
        free(userFuncs);
    }
    userFuncs = p;
    totalUserFuncs += EXPAND_SIZE_CONSTS;
}
/* ---------------------------------------- */

/* ---------------- DISPLAY ---------------- */
void NumConsts_display(FILE *fout)
{
    unsigned int i;
    for (i = 0; i < currNumConsts; i++)
        fprintf(fout, "%g\n", numConsts[i]);
}
void StrConsts_display(FILE *fout)
{
    unsigned int i;
    for (i = 0; i < currStrConsts; i++)
        fprintf(fout, "%lu\n%s\n", strlen(strConsts[i]), strConsts[i]);
}
void LibFuncs_display(FILE *fout)
{
    unsigned int i;
    for (i = 0; i < currLibFuncs; i++)
        fprintf(fout, "%s\n", libFuncs[i]);
}
void UserFuncs_display(FILE *fout)
{
    unsigned int i;
    for (i = 0; i < currUserFuncs; i++)
        fprintf(fout, "%u %u %u %s\n", userFuncs[i].address, userFuncs[i].args, userFuncs[i].localSize, userFuncs[i].id);
}
void Consts_display(FILE *fout)
{
    fprintf(fout, "S:%u\n", currStrConsts);
    StrConsts_display(fout);
    fprintf(fout, "N:%u\n", currNumConsts);
    NumConsts_display(fout);
    fprintf(fout, "U:%u\n", currUserFuncs);
    UserFuncs_display(fout);
    fprintf(fout, "L:%u\n", currLibFuncs);
    LibFuncs_display(fout);
}
/* ----------------------------------------- */

/* ---------------- FREE ---------------- */
void NumConsts_free(void)
{
    if (numConsts)
    {
        free(numConsts);
        numConsts = NULL;
    }
}
void StrConsts_free(void)
{
    if (strConsts)
    {
        unsigned int i;
        for (i = 0; i < currStrConsts; i++)
        {
            if (strConsts[i])
                free(strConsts[i]);
        }
        free(strConsts);
        strConsts = NULL;
    }
}
void LibFuncs_free(void)
{
    if (libFuncs)
    {
        unsigned int i;
        for (i = 0; i < currLibFuncs; i++)
        {
            if (libFuncs[i])
                free(libFuncs[i]);
        }
        free(libFuncs);
        libFuncs = NULL;
    }
}
void UserFuncs_free(void)
{
    if (userFuncs)
    {
        unsigned int i;
        for (i = 0; i < currUserFuncs; i++)
        {
            if (userFuncs[i].id)
                free(userFuncs[i].id);
        }
        free(userFuncs);
        userFuncs = NULL;
    }
}
void Consts_free(void)
{
    NumConsts_free();
    StrConsts_free();
    LibFuncs_free();
    UserFuncs_free();
}
/* -------------------------------------- */