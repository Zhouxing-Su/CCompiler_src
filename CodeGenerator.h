#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>

#include "Symbol.h"

class CodeGenerator
{
    friend int init( int argc, char **argv );
public:
    //static const char type[3][3];

    // the result type will always be the same as the first operand
    std::string* emitExpression( const char *op,
        const Variable::Expression &a, const Variable::Expression &b );

    static CodeGenerator *cg;

private:
    CodeGenerator( const std::string &filename = "IR" );
    ~CodeGenerator();

    int tempVarCount;

    FILE *pfdef;    // pointer to the file storing the definitions of the Intermidiate Repressentation
    FILE *pfimp;    // pointer to the file storing the implementations of the  Intermidiate Repressentation
};

#endif