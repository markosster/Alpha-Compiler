#include "scopespaces.h"

unsigned int programVarsOffset = 0;
unsigned int functionLocalsOffset = 0;
unsigned int formalArgsOffset = 0;
unsigned int scopeSpaceCounter = 1;
unsigned int globals = 0;

const char *scopeSpace_toString(ScopeSpaceType type)
{
    switch(type)
    {
        case program_var:
            return "program_var";
        case function_local:
            return "function_local";
        case formal_arg:
            return "formal_arg";
        default:
            return "none";
    }
}

ScopeSpaceType currScopeSpace(void)
{
    if (scopeSpaceCounter == 1)
        return program_var;
    else
    {
        if (scopeSpaceCounter % 2 == 0)
            return formal_arg;
        else
            return function_local;
    }
}

unsigned int currScopeOffset(void)
{
    switch (currScopeSpace())
    {
    case program_var:
        return programVarsOffset;
    case function_local:
        return functionLocalsOffset;
    case formal_arg:
        return formalArgsOffset;
    default:
        assert(0);
    }
}

void inCurrScopeOffset(void)
{
    switch (currScopeSpace())
    {
    case program_var:
    {
        ++globals;
        ++programVarsOffset;
        break;
    }
    case function_local:
        ++functionLocalsOffset;
        break;
    case formal_arg:
        ++formalArgsOffset;
        break;
    default:
        assert(0);
    }
}

void enterScopeSpace(void) { ++scopeSpaceCounter; }

void exitScopeSpace(void)
{
    assert(scopeSpaceCounter > 1);
    --scopeSpaceCounter;
}

void resetProgramVarsOffset(void) { programVarsOffset = 0; }

void resetFunctionLocalsOffset(void) { functionLocalsOffset = 0; }

void resetFormalArgsOffset(void) { formalArgsOffset = 0; }

void restoreCurrScopeOffset(int offset)
{
    switch (currScopeSpace())
    {
    case program_var:
        programVarsOffset = offset;
        break;
    case function_local:
        functionLocalsOffset = offset;
        break;
    case formal_arg:
        formalArgsOffset = offset;
        break;
    default:
        assert(0);
    }
}