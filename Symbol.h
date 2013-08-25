#ifndef SYMBOL_H
#define SYMBOL_H 0

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <typeinfo>
#include <sstream>

class SymbolTable;

class Symbol
{
public:
    enum IDstate { OK, DUPLICATE, OVERRIDE, CONFLICT }; // only CONFLICT is a bad state

    Symbol( const std::string &name );
    virtual ~Symbol();

    // attach the symbol to the stack and symbol table
    virtual IDstate attach( SymbolTable *st ) const = 0;
    virtual bool isEqual( const Symbol *symbol ) const {
        return ( name == symbol->name );
    }

    bool isSameScope( const Symbol *symbol ) const {
        return ( this->scope == symbol->scope );
    }

    std::string getName() const {
        return name;
    }

protected:
    mutable SymbolTable *scope; // be set when attached to a symbol table

private:
    std::string name;
};


class VarType : public Symbol
{
    friend int init( int argc, char **argv );

public:
    enum AtomTypesIndex { CHAR = 0, INT, LONG, SHORT, FLOAT, DOUBLE, SIGNED, UNSIGNED, VOID };
    enum CompoundLevel { NONE, UNION, STRUCT, ENUM };
    static const int WORD_LENGTH = sizeof(void*);

    // the size is auto-calculated by memberList, auto-fix the compoundLevel by depth and width
    VarType( const std::string &name, int depth, CompoundLevel compoundLevel,
        std::vector<int> *width, SymbolTable *memberList );
    // constructor for basic types
    VarType( const std::string &name, int size );

    ~VarType();

    IDstate attach( SymbolTable *st ) const;
    // not like the function or variable, the same type does not need the same name
    bool isEqual( const Symbol *symbol ) const; // so the names won't be checked
    CompoundLevel getCompoundLevel();
    SymbolTable *getMemberList();
    
    // search the type from current scope to global scope, not include the atomTypes
    static VarType *find( const std::string &name );    // find only by name

    static std::vector<VarType*> stack;     // store defined types till current scope
    static std::vector<VarType*> atomTypes; // store atom types

private:
    // add atom types like "int" to the VarType::atomTypes
    static void initAtomTypes();
    CompoundLevel compoundLevel;
    int size;   // size in byte ( of a single element if it is an array )
    int depth;  // a positive number indicating the depth for a pointer, else it should be 0.
    std::vector<int> *width;    // store width of every dimension for an array
    SymbolTable *memberList;    // the elements are all VarType* in the memberList->svector
};


class Variable : public Symbol
{
public:
    Variable( const std::string &name, VarType* type );
    ~Variable();

    IDstate attach( SymbolTable *st ) const;
    bool isEqual( const Symbol *symbol ) const;

    static Variable *find( const std::string &name );    // find only by name

    static std::vector<Variable*> stack;    // store global or loacal variables

private:
    VarType* type;
    //isGlobal; // distinguish whether its storage on stack or data segment
    int addr;  // shift of the start address in data segment or stack segment from the base 
               // ( but not the top of the stack )
};


class Function : public Symbol
{
public:
    // size and entry will be set according to the function body
    Function( const std::string &name, VarType *returnType, SymbolTable *argList );
    ~Function();

    IDstate attach( SymbolTable *st ) const;
    bool isEqual( const Symbol *symbol ) const;
    
    static Function *find( const std::string &name );    // find only by name

    static std::vector<Function*> stack;     // store all functions ( global scope only )

private:
    VarType* returnType;
    SymbolTable *argList;   // the elements are all VarType* in the argList->svector
    int entry;  // the start address of the function in code segment
    int size;
};


class Label : public Symbol // include start address of 'for', 'while', 'do-while', 'case', 'default' and goto-label
{                           // and the address of the first instruction after loops, function calls and 'if-else'
    friend int init( int argc, char **argv );

public:
    // constructor for loop structures, auto-generating names by current index of the Label::stack
    // because they don't have a name assigned by programmer like goto-label
    Label( int addr );  
    // constructor for goto-label
    Label( const std::string &name, int addr );
    ~Label();

    IDstate attach( SymbolTable *st ) const;
    static Label *find( const std::string &name );    // find only by name

    static std::vector<Label*> stack;    // store all labels ( in current scope & cannot be global )

private:
    static const std::string autoNamePrefix;
    static std::string autoName;    // used in constructor 'Label( int addr )'
    static int loopCount;

    int addr;
};

#endif