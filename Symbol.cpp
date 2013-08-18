#include "Symbol.h"

using namespace std;

// class Symbol

Symbol::Symbol( const string &n ) : name(n) {
    
}

Symbol::~Symbol() {

}

// class VarType

vector<VarType*> VarType::stack;

VarType::VarType( const string &n, int s, int c, vector<VarType*> ml )
    : Symbol(n), size(s), count(c), memberList(ml) {

}

VarType::~VarType() {

}

// class Variable

vector<Variable*> Variable::stack;
vector<Variable*> Variable::heap;

Variable::Variable( const string &n, VarType* t, int a, int c )
    : Symbol(n), type(t), addr(a), count(c) {

}

Variable::~Variable() {

}

// class Function

vector<Function*> Function::list;

Function::Function( const string &n ) : Symbol(n) {

}

Function::~Function() {

}

// class Label

vector<Label*> Label::list;
char Label::nameBuf[NAME_BUF_SIZE];

Label::Label( int a ) : Symbol( itoa(list.size(), nameBuf, 16 ) ), addr(a) {

}


Label::Label( const string &n, int a ) : Symbol(n), addr(a) {

}

Label::~Label() {

}