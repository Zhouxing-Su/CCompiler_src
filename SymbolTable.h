#include <vector>
#include "Symbol.h"

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H 0

class SymbolTable
{
public:
    SymbolTable(void);
    virtual ~SymbolTable(void);

    /**
        push a SymbolTable into the SymbolTable::stack when
        entering a new scope. the top of the stack will
        always be the SymbolTable for current scope.
        search the SymbolTable::stack from top to bottom 
        will get SymbolTable of outter and outter scopes step by step.
    */
    static std::vector<SymbolTable*> stack;

private:
    std::vector<Symbol*> svector;
};

#endif