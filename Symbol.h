#ifndef SYMBOL_H
#define SYMBOL_H 0

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

class SymbolTable;

class Symbol
{
public:
    //enum SymboleType { VAR, FUNC, TYPE };

    Symbol( const std::string &name );
    virtual ~Symbol();

    // attach the symbol to the stack and symbol table
    virtual int attach( SymbolTable *st ) const = 0;

    std::string &getName() {
        return name;
    }


private:
    std::string name;
    //SymboleType type;
};

class VarType : public Symbol
{
    friend int init( int argc, char **argv );

public:
    // the size is auto-calculated by memberList
    VarType( const std::string &name, int depth, 
        std::vector<int> *width, std::vector<VarType*> *memberList );
    // constructor for basic types
    VarType( const std::string &name, int size );

    ~VarType();

    virtual int attach( SymbolTable *st ) const;
    
    // search the type from current scope to global scope, not include the atomTypes
    static VarType *find( const std::string &name );

    static std::vector<VarType*> stack;     // store defined types till current scope
    static std::vector<VarType*> atomTypes; // store atom types

    enum AtomTypesIndex { CHAR = 0, INT, LONG, SHORT, FLOAT, DOUBLE, SIGNED, UNSIGNED, VOID };
    static const int WORD_LENGTH = sizeof(void*);

private:
    // add atom types like "int" to the VarType::atomTypes
    static void initAtomTypes();
    int size;   // size in byte ( of a single element if it is an array )
    int depth;  // a positive number indicating the depth for a pointer, else it should be 0.
    std::vector<int> *width;    // store width of every dimension for an array
    std::vector<VarType*> *memberList;
};



class Variable : public Symbol
{
public:
    Variable( const std::string &name, VarType* type, int addr );
    ~Variable();

    int attach( SymbolTable *st ) const;

    static std::vector<Variable*> stack;    // store loacal variables
    static std::vector<Variable*> heap;     // store global variables

private:
    VarType* type;
    //isGlobal; // distinguish whether its storage on stack or data segment
    int addr;  // shift of the start address in data segment or stack segment from the base ( but not the top of the stack )
    //int size; // this field is replaced because it can be found in its type
    //int count;  // if it is not an array, this field should be 1.
};

class Function : public Symbol
{
public:
    Function( const std::string &name );
    ~Function();

    int attach( SymbolTable *st ) const;

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

    int attach( SymbolTable *st ) const;

    static std::vector<Label*> stack;    // store all labels ( in current scope & cannot be global )

private:
    // stores a hexical address which length fits the word size of your processer
    // e.g. a 32-bits processer may get 2*4=8 chars to express 00000000~FFFFFFFF
    static const int NAME_BUF_SIZE = ( ( 2*VarType::WORD_LENGTH )+1 ); // one char for '\0'
    static char nameBuf[NAME_BUF_SIZE]; // used in constructor 'Label( int addr )'
    static char nameFormat[8];          // control the generation of the prefixed zeros
    
    // set Label::nameBuf to all '0' and set Label::nameFormat according to NAME_BUF_SIZE
    static void initNameBuf();

    std::string name;
    int addr;
};

#endif