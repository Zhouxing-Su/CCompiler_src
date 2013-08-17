#include <string>
#include <vector>

#ifndef SYMBOL_H
#define SYMBOL_H 0

class Symbol
{
public:
    enum SymboleType { VAR, FUNC, TYPE };

    Symbol(void);
    virtual ~Symbol(void);

private:
    std::string name;
    SymboleType type;
};

class VarType : public Symbol
{
public:
    VarType();
    ~VarType();

    static std::vector<VarType*> stack;     // store defined types till current scope

private:
    int size;   // size in byte ( of a single element if it is an array )
    int count;  // if it is not an array, this field should be 1.
    std::vector<VarType*> memberList;
};



class Variable : public Symbol
{
public:
    Variable();
    ~Variable();

    static std::vector<Variable*> stack;    // store loacal variables
    static std::vector<Variable*> heap;     // store global variables

private:
    VarType* type;
    //isGlobal; // distinguish whether its storage on stack or data segment
    int shift;  // shift of the start address in data segment or stack segment
    //int size; // find in its type
    int count;  // if it is not an array, this field should be 1.
};

class Function : public Symbol
{
public:
    Function();
    ~Function();

    static std::vector<Function*> list;     // store all functions ( only global )

private:
    VarType* returnType;
    std::vector<Variable*> argList;
    int shift;  // shift of the start address in code segment
    int size;
};

#endif