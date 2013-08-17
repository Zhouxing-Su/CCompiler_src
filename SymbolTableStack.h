/**
    push a SymbolTable into the SymbolTableStack when
    entering a new scope. the top of the stack will
    always be the SymbolTable for current scope.
    search the SymbolTableStack from top to bottom 
    will get SymbolTable of outter and outter scopes step by step.
*/

#include <stack>
#include "SymbolTable.h"

#ifndef SYMBOL_TABLE_STACK_H
#define SYMBOL_TABLE_STACK_H 0

class SymbolTableStack
{
public:
    friend int init( int argc, char **argv );

    static SymbolTableStack *sts;

private:
    // the constructor should not be called outside the main.cpp::init() function
    SymbolTableStack(void);
    virtual ~SymbolTableStack(void);

    std::stack<SymbolTable*> ststack;
};

#endif