#include "CodeGenerator.h"

using namespace std;

//const char CodeGenerator::type[3][3] = { "", "23", "" };
CodeGenerator* CodeGenerator::cg;

CodeGenerator::CodeGenerator( const std::string &filename ) : tempVarCount(0) {
    string fn( filename + "/IR.def" );
    pfdef = fopen( fn.c_str(), "w" );
    fn = filename + "/IR.imp";
    pfimp = fopen( fn.c_str(), "w" );
}

CodeGenerator::~CodeGenerator() {
    fclose( pfdef );
    fclose( pfimp );
}

std::string* CodeGenerator::emitExpression( const char *op,
        const Variable::Expression &a, const Variable::Expression &b ) {
    char result[Symbol::MaxNameLen];
    char resultName[Symbol::MaxNameLen];
    char e1[Symbol::MaxNameLen];
    char e2[Symbol::MaxNameLen];

    sprintf( resultName, "@t%d", tempVarCount++ );

    switch( a.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( e1, "#%d:c1", a.value.c );
        sprintf( result, "%%%s:c1", resultName );
        break;
    case VarType::ExprType::INTEGER:
        sprintf( e1, "#%d:i4", a.value.i );
        sprintf( result, "%%%s:i4", resultName );
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( e1, "#%u:u4", a.value.u );
        sprintf( result, "%%%s:u4", resultName );
        break;
    case VarType::ExprType::REAL:
        sprintf( e1, "#%lf:f8", a.value.d );
        sprintf( result, "%%%s:f8", resultName );
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( e1, "#%s:s%d", a.value.s->c_str(), a.value.s->size() );
        sprintf( result, "%%%s:s%d", resultName, a.value.s->size() );
        break;
    case VarType::ExprType::VAR:
        sprintf( e1, "%%%s:%s", a.pvar->getName().c_str(), a.pvar->getTypeName().c_str() );
        sprintf( result, "%%%s:%s", resultName, a.pvar->getTypeName().c_str() );
        break;
    default:
        ;
    }

    switch( b.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( e2, "#%d:c1", b.value.c );
        break;
    case VarType::ExprType::INTEGER:
        sprintf( e2, "#%d:i4", b.value.i );
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( e2, "#%u:u4", b.value.u );
        break;
    case VarType::ExprType::REAL:
        sprintf( e2, "#%lf:f8", b.value.d );
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( e2, "#%s:s%d", b.value.s->c_str(), b.value.s->size() );
        break;
    case VarType::ExprType::VAR:
        sprintf( e2, "%%%s:%s", b.pvar->getName().c_str(), b.pvar->getTypeName().c_str() );
        break;
    default:
        ;
    }

    fprintf( pfimp, "%s %s, %s, %s\n", op, result, e1, e2 );
    return new std::string(resultName);
}
