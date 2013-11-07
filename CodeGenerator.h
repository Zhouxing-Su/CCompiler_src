#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>

#include "Symbol.h"
#include "SymbolTable.h"

class CodeGenerator
{
    friend int init( int argc, char **argv );
public:
    //static const char type[3][3];
    typedef enum ResultTypeSwitch { 
        SAME_AS_FIRST_OPRAND, SAME_AS_SECOND_OPRAND,
        BOOL_VALUE
    } ResultTypeSwitch;

    typedef enum LogicOperation { AND=0, OR=1 } LogicOperation;

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
    CodeGenerator( const std::string &filename = "IR" );
    ~CodeGenerator();

    int tempVarCount;

    FILE *pfdef;    // pointer to the file storing the definitions of the Intermidiate Repressentation
    FILE *pfimp;    // pointer to the file storing the implementations of the  Intermidiate Repressentation
};

#endif