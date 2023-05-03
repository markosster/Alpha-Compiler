#ifndef CONSTS_H
#define CONSTS_H

#include "instructions.h"

#define EXPAND_SIZE_CONSTS 1024

typedef struct UserFunc
{
    unsigned int address;
    unsigned int localSize;
    unsigned int args;
    char *id;
} UserFunc;

/* Constant Tables */
extern double *numConsts;
extern unsigned int totalNumConsts;
extern unsigned int currNumConsts;
#define CURR_SIZE_NUMCONSTS (totalNumConsts * sizeof(double))
#define NEW_SIZE_NUMCONSTS (EXPAND_SIZE_CONSTS * sizeof(double) + CURR_SIZE_NUMCONSTS)

extern char **strConsts;
extern unsigned int totalStrConsts;
extern unsigned int currStrConsts;
#define CURR_SIZE_STRCONSTS (totalStrConsts * sizeof(char *))
#define NEW_SIZE_STRCONSTS (EXPAND_SIZE_CONSTS * sizeof(char *) + CURR_SIZE_STRCONSTS)

extern char **libFuncs;
extern unsigned int totalLibFuncs;
extern unsigned int currLibFuncs;
#define CURR_SIZE_LIBFUNCS (totalLibFuncs * sizeof(char *))
#define NEW_SIZE_LIBFUNCS (EXPAND_SIZE_CONSTS * sizeof(char *) + CURR_SIZE_LIBFUNCS)

extern struct UserFunc *userFuncs;
extern unsigned int totalUserFuncs;
extern unsigned int currUserFuncs;
#define CURR_SIZE_USERFUNCS (totalUserFuncs * sizeof(struct UserFunc))
#define NEW_SIZE_USERFUNCS (EXPAND_SIZE_CONSTS * sizeof(struct UserFunc) + CURR_SIZE_USERFUNCS)
/* ------------ */

/* Functions to insert into a const table  */
unsigned int NumConsts_add(double num);
unsigned int StrConsts_add(char *str);
unsigned int LibFuncs_add(char *libId);
unsigned int UserFuncs_add(UserFunc uf);

/* Getter functions for retrieving data from Constant Tables */
double NumConsts_get(unsigned int index);
char *StrConsts_get(unsigned int index);
char *LibFuncs_get(unsigned int index);
UserFunc *UserFuncs_get(unsigned int index);

/* Expand table size for a const table */
void NumConsts_expand(void);
void StrConsts_expand(void);
void LibFuncs_expand(void);
void UserFuncs_expand(void);

/* Prints */
void StrConsts_display(FILE *fout);
void NumConsts_display(FILE *fout);
void LibFuncs_display(FILE *fout);
void UserFuncs_display(FILE *fout);

/* Display all tables */
void Consts_display(FILE *fout);

/* Free Tables */
void NumConsts_free(void);
void StrConsts_free(void);
void LibFuncs_free(void);
void UserFuncs_free(void);

/* Free all tables */
void Consts_free(void);

#endif