#include "SymbolTable.h"


std::vector<SymbolTable*> SymbolTable::stack;

SymbolTable::SymbolTable(void) {
}

SymbolTable::SymbolTable( int num, Symbol *symbol ) 
    : svector( num, symbol ) {

}

SymbolTable::~SymbolTable(void) {
}

int SymbolTable::addSymbol( Symbol *sym ) {
    svector.push_back( sym );
    return 0;
}

int SymbolTable::appendTable( SymbolTable *st ) {
    for( int i = 0 ; i < st->size() ; ++i ) {
        this->svector.push_back( (*st)[i] );
    }
    
    return 0;
}


Symbol * SymbolTable::operator[]( int index ) {
    return svector[index];
}

Symbol * SymbolTable::at( int index ) const {
    return svector.at (index );
}


int SymbolTable::size() {
    return svector.size();
}

bool SymbolTable::isEqual( SymbolTable *pst ) const {
    for( int i = 0 ; i < pst->size() ; ++i ) {
        if( !( this->at(i)->isEqual( pst->at(i) ) ) ) {
            return false;
        }
    }
    return true;
}

void SymbolTable::initSymbolTableStack() {
    stack.push_back( new SymbolTable() );
}

SymbolTable * SymbolTable::getCurrentScope() {
    return stack.back();
}

