#include <vector>
#include "Symbol.h"

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H 0

class SymbolTable
{
public:
    SymbolTable(void);
    virtual ~SymbolTable(void);

private:
    std::vector<Symbol*> svector;
};

#endif