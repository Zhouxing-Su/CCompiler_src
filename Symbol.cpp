#include "SymbolTable.h"

using namespace std;

// class Symbol

Symbol::Symbol( const string &n ) : name(n) {
    
}

Symbol::~Symbol() {

}

// class VarType

vector<VarType*> VarType::stack;
vector<VarType*> VarType::atomTypes;

VarType::VarType( const string &n, int d, CompoundLevel cl, vector<int> *w, SymbolTable *ml )
    : Symbol(n), depth(d), compoundLevel(cl), width(w), memberList(ml) {
    size = 0;
    if ( cl == CompoundLevel::UNION ) { // find the size of the largest one
        for( int i = 0 ; i < ml->size() ; ++i ) {
            if ( size < dynamic_cast<VarType*>(ml->at(i))->size ) {
                size = dynamic_cast<VarType*>(ml->at(i))->size;
            }
        }
    } else {    // calculate the sum of every elements' size
        for( int i = 0 ; i < ml->size() ; ++i ) {
            size += dynamic_cast<VarType*>(ml->at(i))->size;
        }
    }
}

VarType::VarType( const string &n, int s )
    : Symbol(n), depth(0), compoundLevel(CompoundLevel::NONE), size(s), width(NULL), memberList(NULL) {

}

VarType::~VarType() {

}

int VarType::attach( SymbolTable *st ) const {
    stack.push_back( const_cast<VarType*>(this) );
    st->addSymbol( const_cast<VarType*>(this) );
    return 0;
}

VarType::CompoundLevel VarType::getCompoundLevel() {
    return compoundLevel;
}


SymbolTable * VarType::getMemberList() {
    return memberList;
}


VarType * VarType::find( const std::string &name ) {
    for( int i = stack.size()-1 ; i >= 0 ; --i ) {
        if( name.compare( stack[i]->getName() ) == 0 ) {
            return stack[i];
        }
    }
    return NULL;
}

void VarType::initAtomTypes() {
    atomTypes.push_back( new VarType( "char", 1 ) );
    atomTypes.push_back( new VarType( "int", WORD_LENGTH ) );
    atomTypes.push_back( new VarType( "long", WORD_LENGTH ) );
    atomTypes.push_back( new VarType( "short", WORD_LENGTH/2 ) );
    atomTypes.push_back( new VarType( "float", WORD_LENGTH ) );
    atomTypes.push_back( new VarType( "double", WORD_LENGTH*2 ) );
    atomTypes.push_back( new VarType( "signed", WORD_LENGTH ) );
    atomTypes.push_back( new VarType( "unsigned", WORD_LENGTH ) );
    atomTypes.push_back( new VarType( "void", 0 ) );
}


// class Variable

vector<Variable*> Variable::stack;
vector<Variable*> Variable::heap;

Variable::Variable( const string &n, VarType* t, int a )
    : Symbol(n), type(t), addr(a) {

}

Variable::~Variable() {

}

int Variable::attach( SymbolTable *st ) const {
    return 0;
}


// class Function

vector<Function*> Function::list;

Function::Function( const string &n ) : Symbol(n) {

}

Function::~Function() {

}

int Function::attach( SymbolTable *st ) const {
    return 0;
}


// class Label

vector<Label*> Label::stack;
char Label::nameBuf[NAME_BUF_SIZE];
char Label::nameFormat[8];

void Label::initNameBuf() {
    memset( nameBuf, '0', NAME_BUF_SIZE );
    sprintf( nameFormat, "%%0%dh", NAME_BUF_SIZE );
}

Label::Label( int a ) : Symbol( nameBuf ), addr(a) {
    sprintf( nameBuf, nameFormat, stack.size() );    // prepare next label name for loop structure
}


Label::Label( const string &n, int a ) : Symbol(n), addr(a) {

}

Label::~Label() {

}

int Label::attach( SymbolTable *st ) const {
    return 0;
}
