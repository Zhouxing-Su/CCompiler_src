#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>

#include "Symbol.h"
#include "SymbolTable.h"
#include "Log.h"

class CodeGenerator
{
    friend int init( int argc, char **argv );
public:
    typedef enum ResultTypeSwitch { 
        SAME_AS_FIRST_OPRAND, SAME_AS_SECOND_OPRAND,
        BOOL_VALUE
    } ResultTypeSwitch;
    typedef enum LogicOperation { AND=0, OR=1 } LogicOperation;

    typedef enum MemTypeIndex { C1=0, I8, I4, I2, U8, U4, U2, F8, F4, STR } MemTypeIndex;
    typedef enum RegTypeIndex { DL=0, DX, EDX, EDXEAX, ST, ESI } RegTypeIndex;

    static const int typeNum = 10;
    static const int regTypeNum = 6;
    static const int opTypeNum = 2;
    static char const reg[typeNum][typeNum][8];
    static char const read_a[typeNum][typeNum][8];
    static char const opPrefix[opTypeNum][typeNum][typeNum][8];
    static char const write_r[typeNum][regTypeNum][32];

    const std::string folder;

    void generateASM();
    void emitLoad_a(MemTypeIndex rtype, MemTypeIndex atype, std::string *aName);
    void emitOperation();
    void emitWrite_r(MemTypeIndex rtype, RegTypeIndex regtype, std::string *rName);


    void emitBlock( char block );

    // the result type will always be the same as the first operand or boolean
    std::string* emitExpression( const char *op, ResultTypeSwitch resultType,
        const Variable::Expression &a, const Variable::Expression &b );

    // "i++" is:    MOV t1, i
    //              INC i
    // resultName is t1
    std::string* emitInc( const char *op, Variable::Expression lvar );

    // "a&&b" is:   NEQ t1, a, 0
    //              JCF LABEL_FALSE, t1
    //              NEQ t1, b, 0
    //              LABEL_FALSE:
    // resultName is t1
    // "a||b" is:   NEQ t1, a, 0
    //              JCT LABEL_TRUE, t1
    //              NEQ t1, b, 0
    //              LABEL_TRUE:
    // resultName is t1
    std::string* emitAndOr( LogicOperation lop, const Variable::Expression &a, const Variable::Expression &b );

    void emitMove( const Variable::Expression &r, const Variable::Expression &a );
    void emitAssignment( const char *op, const Variable::Expression &a, const Variable::Expression &b );
    void emitBranch( const char *op, const char *labelName, const Variable::Expression &c );
    void emitJump( const char *labelName );
    void emitLabel( const char* labelName );

    static char* getFullName( const Variable::Expression e, char *fullName );

    static CodeGenerator *cg;

private:
    CodeGenerator( const std::string &folder = "TestCase" );
    ~CodeGenerator();

    int tempVarCount;

    FILE *pfdef;    // pointer to the file storing the definitions of the Intermidiate Repressentation
    FILE *pfimp;    // pointer to the file storing the implementations of the Intermidiate Repressentation
    FILE *pfasm;    // pointer to the target asm file
};

#endif