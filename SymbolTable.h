
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H 0

#include <vector>
#include "Symbol.h"

class SymbolTable
{
    friend int init( int argc, char **argv );

public:
    SymbolTable();
    SymbolTable( int num, Symbol *symbol );
    virtual ~SymbolTable();

    int addSymbol( Symbol *symbol );
    int appendTable( SymbolTable *symbolTable );
    Symbol *operator[]( int index );
    Symbol *at( int index );
    int size();

    // return the top element of the symbol table stack
    static SymbolTable *getCurrentScope();

    /**
        push a SymbolTable into the SymbolTable::stack when
        entering a new scope. the top of the stack will
        always be the SymbolTable for current scope.
        search the SymbolTable::stack from top to bottom 
        will get SymbolTable of outter and outter scopes step by step.
    */
    static std::vector<SymbolTable*> stack;

private:
    // push the global symbol table (ST0) into the stack
    static void initSymbolTableStack();

    std::vector<Symbol*> svector;
};

#endif