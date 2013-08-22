#include "SymbolTable.h"


std::vector<SymbolTable*> SymbolTable::stack;

SymbolTable::SymbolTable(void) {
}


SymbolTable::~SymbolTable(void) {
}

int SymbolTable::addSymbol( const Symbol *sym ) {
    svector.push_back( const_cast<Symbol*>(sym) );
    return 0;
}

void SymbolTable::initSymbolTableStack() {
    stack.push_back( new SymbolTable() );
}

SymbolTable * SymbolTable::getCurrentScope() {
    return stack.back();
}

