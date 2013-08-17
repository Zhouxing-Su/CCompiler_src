#include "Symbol.h"

// class Symbol

Symbol::Symbol(void)
{
}

Symbol::~Symbol(void)
{
}

// class VarType

std::vector<VarType*> VarType::stack;

VarType::VarType()
{
}

VarType::~VarType()
{
}

// class Variable

std::vector<Variable*> Variable::stack;
std::vector<Variable*> Variable::heap;

Variable::Variable()
{
}

Variable::~Variable()
{
}

// class Function

std::vector<Function*> Function::list;

Function::Function()
{
}

Function::~Function()
{
}
