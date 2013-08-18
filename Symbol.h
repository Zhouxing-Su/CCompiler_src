#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifndef SYMBOL_H
#define SYMBOL_H 0

class Symbol
{
public:
    //enum SymboleType { VAR, FUNC, TYPE };

    Symbol( const std::string &name );
    virtual ~Symbol();

private:
    std::string name;
    //SymboleType type;
};

class VarType : public Symbol
{
public:
    VarType( const std::string &name, int size, int count, std::vector<VarType*> memberList );
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
    Variable( const std::string &name, VarType* type, int addr, int count );
    ~Variable();

    static std::vector<Variable*> stack;    // store loacal variables
    static std::vector<Variable*> heap;     // store global variables

private:
    VarType* type;
    //isGlobal; // distinguish whether its storage on stack or data segment
    int addr;  // shift of the start address in data segment or stack segment from the base ( but not the top of the stack )
    //int size; // this field is replaced because it can be found in its type
    int count;  // if it is not an array, this field should be 1.
};

class Function : public Symbol
{
public:
    Function( const std::string &name );
    ~Function();

    static std::vector<Function*> list;     // store all functions ( global scope only )

private:
    VarType* returnType;
    std::vector<Variable*> argList;
    int entry;  // the start address of the function in code segment
    int size;
};

class Label : public Symbol // include start address of 'for', 'while', 'do-while' and goto-label
{
    friend int init( int argc, char **argv );

public:
    // constructor for loop structures, auto-generating names by current index of the Label::list
    // because they don't have a name assigned by programmer like goto-label
    Label( int addr );  
    // constructor for goto-label
    Label( const std::string &name, int addr );
    ~Label();

    static std::vector<Label*> stack;    // store all labels ( in current scope & cannot be global )

private:
    // stores a hexical address which length fits the word size of your processer
    // e.g. a 32-bits processer may get 2*4=8 chars to express 00000000~FFFFFFFF
    static const int NAME_BUF_SIZE = ( ( 2*sizeof(void*) )+1 ); // one char for '\0'
    static char nameBuf[NAME_BUF_SIZE]; // used in constructor 'Label( int addr )'
    static char nameFormat[8];          // control the generation of the prefixed zeros
    
    static void initNameBuf();

    std::string name;
    int addr;
};

#endif