#ifndef SCOPESPACES_H
#define SCOPESPACES_H

#include "assert.h"
#include "offsetstack.h"

typedef enum ScopeSpaceType
{
    program_var,
    function_local,
    formal_arg,
    none
} ScopeSpaceType;

extern unsigned int programVarsOffset;
extern unsigned int functionLocalsOffset;
extern unsigned int formalArgsOffset;
extern unsigned int scopeSpaceCounter;
extern unsigned int globals;

ScopeSpaceType currScopeSpace(void);
unsigned int currScopeOffset(void);
void inCurrScopeOffset(void);
void enterScopeSpace(void);
void exitScopeSpace(void);
void restoreCurrScopeOffset(int offset);

void resetProgramVarsOffset(void);
void resetFunctionLocalsOffset(void);
void resetFormalArgsOffset(void);

const char *scopeSpace_toString(ScopeSpaceType type);

#endif