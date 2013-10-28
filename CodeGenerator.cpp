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

std::string* CodeGenerator::emitExpression( const char *op, int resultType,
        const Variable::Expression &a, const Variable::Expression &b ) {
    char result[Symbol::MaxNameLen];
    char resultName[Symbol::MaxNameLen];
    char e1[Symbol::MaxNameLen];
    char e2[Symbol::MaxNameLen];

    sprintf( resultName, "@t%d", tempVarCount++ );

    switch( a.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( e1, "#%d:c1", a.value.c );
        if( resultType == 0 ) {
            sprintf( result, "%%%s:c1", resultName );
        }
        break;
    case VarType::ExprType::INTEGER:
        sprintf( e1, "#%d:i4", a.value.i );
        if( resultType == 0 ) {
            sprintf( result, "%%%s:i4", resultName );
        }
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( e1, "#%u:u4", a.value.u );
        if( resultType == 0 ) {
            sprintf( result, "%%%s:u4", resultName );
        }
        break;
    case VarType::ExprType::REAL:
        sprintf( e1, "#%lf:f8", a.value.d );
        if( resultType == 0 ) {
            sprintf( result, "%%%s:f8", resultName );
        }
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( e1, "#%s:s%d", a.value.s->c_str(), a.value.s->size() );
        if( resultType == 0 ) {
            sprintf( result, "%%%s:s%d", resultName, a.value.s->size() );
        }
        break;
    case VarType::ExprType::VAR:
        sprintf( e1, "%%%s:%s", a.pvar->getName().c_str(), a.pvar->getTypeName().c_str() );
        if( resultType == 0 ) {
            sprintf( result, "%%%s:%s", resultName, a.pvar->getTypeName().c_str() );
        }
        break;
    default:
        ;
    }

    switch( b.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( e2, "#%d:c1", b.value.c );
        if( resultType == 1 ) {
            sprintf( result, "%%%s:c1", resultName );
        }
        break;
    case VarType::ExprType::INTEGER:
        sprintf( e2, "#%d:i4", b.value.i );
        if( resultType == 1 ) {
            sprintf( result, "%%%s:i4", resultName );
        }
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( e2, "#%u:u4", b.value.u );
        if( resultType == 1 ) {
            sprintf( result, "%%%s:u4", resultName );
        }
        break;
    case VarType::ExprType::REAL:
        sprintf( e2, "#%lf:f8", b.value.d );
        if( resultType == 1 ) {
            sprintf( result, "%%%s:f8", resultName );
        }
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( e2, "#%s:s%d", b.value.s->c_str(), b.value.s->size() );
        if( resultType == 1 ) {
            sprintf( result, "%%%s:s%d", resultName, b.value.s->size() );
        }
        break;
    case VarType::ExprType::VAR:
        sprintf( e2, "%%%s:%s", b.pvar->getName().c_str(), b.pvar->getTypeName().c_str() );
        if( resultType == 1 ) {
            sprintf( result, "%%%s:%s", resultName, b.pvar->getTypeName().c_str() );
        }
        break;
    default:
        ;
    }

    fprintf( pfimp, "%s %s, %s, %s\n", op, result, e1, e2 );
    return new std::string(resultName);
}

// "i++" is:   MOV t1, i    ;
            // INC i        ; resultName = t1
std::string* CodeGenerator::emitInc( const char *op, Variable::Expression lvar ) {
    char result[Symbol::MaxNameLen];
    char resultName[Symbol::MaxNameLen];
    char var[Symbol::MaxNameLen];

    sprintf( resultName, "@t%d", tempVarCount++ );

    if( ( lvar.type == VarType::ExprType::VAR ) 
        && ( lvar.pvar->getType().getPointerDepth() > 0 
            || lvar.pvar->getType().getCompoundLevel() == VarType::CompoundLevel::NONE ) ) {
            sprintf( var, "%%%s:%s", lvar.pvar->getName().c_str(), lvar.pvar->getTypeName().c_str() );
    } else {
        return NULL;
    }
    
    sprintf( result, "%%%s:%s", resultName, lvar.pvar->getTypeName().c_str() );
    
    fprintf( pfimp, "MOV %s, %s\n", result, var );
    fprintf( pfimp, "%s %s\n", op, var );
    return new std::string(resultName);
}
