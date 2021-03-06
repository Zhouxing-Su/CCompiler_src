#include "CodeGenerator.h"

using namespace std;

int irparse(void);

CodeGenerator* CodeGenerator::cg;

// i8 and u8("EDX,EAX") can not be right 
char const CodeGenerator::reg[typeNum][typeNum][8] = {
    {"DL","EDX,EAX","EDX","DX","EDX,EAX","EDX","DX","ST","ST","-"},
    {"EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","ST","ST","-"},
    {"EDX","EDX,EAX","EDX","EDX","EDX,EAX","EDX","EDX","ST","ST","-"},
    {"DX","EDX,EAX","EDX","DX","EDX,EAX","EDX","DX","ST","ST","-"},
    {"EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","EDX,EAX","ST","ST","-"},
    {"EDX","EDX,EAX","EDX","EDX","EDX,EAX","EDX","EDX","ST","ST","-"},
    {"DX","EDX,EAX","EDX","DX","EDX,EAX","EDX","DX","ST","ST","-"},
    {"ST","ST","ST","ST","ST","ST","ST","ST","ST","-"},
    {"ST","ST","ST","ST","ST","ST","ST","ST","ST","-"},
    {"-","-","-","-","-","-","-","-","-","ESI"}
};
char const CodeGenerator::read_a[typeNum][typeNum][8] = {
    {"MOV","MOVSX","MOVSX","MOVSX","MOVSX","MOVSX","MOVSX","FILD","FILD","-"},
    {"MOV","MOV","MOV","MOV","MOV","MOV","MOV","FILD","FILD","-"},
    {"MOV","MOVSX","MOV","MOV","MOVSX","MOV","MOV","FILD","FILD","-"},
    {"MOV","MOVSX","MOVSX","MOV","MOVSX","MOVSX","MOV","FILD","FILD","-"},
    {"MOV","MOV","MOV","MOV","MOV","MOV","MOV","FILD","FILD","-"},
    {"MOV","MOV","MOV","MOV","MOVSX","MOV","MOV","FILD","FILD","-"},
    {"MOV","MOV","MOVSX","MOV","MOVSX","MOVSX","MOV","FILD","FILD","-"},
    {"FLD","FLD","FLD","FLD","FLD","FLD","FLD","FLD","FLD","-"},
    {"FLD","FLD","FLD","FLD","FLD","FLD","FLD","FLD","FLD","-"},
    {"-","-","-","-","-","-","-","-","-","LEA"}

};
char const CodeGenerator::opPrefix[opTypeNum][typeNum][typeNum][8] = {
    {   {"","","","","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"FI","FI","FI","FI","FI","FI","FI","F","F","-"},
        {"FI","FI","FI","FI","FI","FI","FI","F","F","-"},
        {"-","-","-","-","-","-","-","-","-","-"}   },
    {   {"","I","I","I","","","","F","F","-"},
        {"I","I","I","I","","I","I","F","F","-"},
        {"I","I","I","I","","","I","F","F","-"},
        {"I","I","I","I","","","","F","F","-"},
        {"","","","","","","","F","F","-"},
        {"","I","","","","","","F","F","-"},
        {"","I","I","","","","","F","F","-"},
        {"FI","FI","FI","FI","FI","FI","FI","F","F","-"},
        {"FI","FI","FI","FI","FI","FI","FI","F","F","-"},
        {"-","-","-","-","-","-","-","-","-","-"}    }
};
char const CodeGenerator::write_r[typeNum][regTypeNum][32] = {
    {"MOV %s,DL","MOV %s, DL","MOV %s, DL","MOV %s,AL","FISTP %s",""},
    {"-","-","-","MOV %s+4, EDX\nMOV %s, EAX","FISTP %s","-"},
    {"-","-","MOV %s, EDX","MOV %s, EAX","FISTP %s","-"},
    {"-","MOV %s, DX","MOV %s, DX","MOV %s, AX","FISTP %s","-"},
    {"-","-","-","MOV %s+4, EDX\nMOV %s, EAX","FISTP %s","-"},
    {"-","-","MOV %s, EDX","MOV %s, EAX","FISTP %s","-"},
    {"-","MOV %s, DX","MOV %s, DX","MOV %s, AX","FISTP %s","-"},
    {"-","-","-","-","FSTP %s","-"},
    {"-","-","-","-","FSTP %s","-"},
    {"-","-","-","-","-","MOV %s,ESI"}
};

CodeGenerator::CodeGenerator( const std::string &f ) : tempVarCount(0), folder(f) {
    string fn( folder + "/IR.def" );
    pfdef = fopen( fn.c_str(), "w" );
    fn = folder + "/IR.imp";
    pfimp = fopen( fn.c_str(), "w" );
}

CodeGenerator::~CodeGenerator() {
    fclose( pfdef );
    fclose( pfimp );
    fclose( pfasm );
}


void CodeGenerator::generateASM()
{
    // flush the buf
    fclose( pfdef );
    fclose( pfimp );

    // handle the output asm file
    string fn( folder + "/target.asm" );
    pfasm = fopen( fn.c_str(), "w" );
    
    // handle the definition and implementation file
    fn = folder + "/IR.def";
    if ( NULL == freopen( fn.c_str(), "r", stdin ) ) {
        Log::FileNotFoundError( fn.c_str() );
        return;
    }

    irparse();
}

void CodeGenerator::emitLoad_a(MemTypeIndex rtype, MemTypeIndex atype, string *aName)
{
    fprintf( pfasm, "%s %s, %s\n", read_a[rtype][atype], reg[rtype][atype], aName->c_str() );
}

void CodeGenerator::emitOperation()
{

}

void CodeGenerator::emitWrite_r(MemTypeIndex rtype, RegTypeIndex regtype, string *rName)
{
    fprintf( pfasm, write_r[rtype][regtype], rName->c_str() );
    fputc( '\n', pfasm );
}


char* CodeGenerator::getFullName( const Variable::Expression e, char *fullName ) {
    switch( e.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( fullName, "#%d:c1", e.value.c );
        break;
    case VarType::ExprType::INTEGER:
        sprintf( fullName, "#%d:i4", e.value.i );
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( fullName, "#%u:u4", e.value.u );
        break;
    case VarType::ExprType::REAL:
        sprintf( fullName, "#%lf:f8", e.value.d );
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( fullName, "#%s:s%d", e.value.s->c_str(), e.value.s->size() );
        break;
    case VarType::ExprType::VAR:
        sprintf( fullName, "%%%s:%s", e.pvar->getName().c_str(), e.pvar->getTypeName().c_str() );
        break;
    default:
        ;
    }
    return fullName;
}



void CodeGenerator::emitBlock( char block )
{
    fprintf( pfimp, "%c\n", block );
}

std::string* CodeGenerator::emitExpression( const char *op, ResultTypeSwitch resultType,
        const Variable::Expression &a, const Variable::Expression &b ) {
    char result[Symbol::MaxNameLen];
    char resultName[Symbol::MaxNameLen];
    char e1[Symbol::MaxNameLen];
    char e2[Symbol::MaxNameLen];

    sprintf( resultName, "@t%d", tempVarCount++ );
    
    if( resultType == ResultTypeSwitch::BOOL_VALUE ) {
        sprintf( result, "%%%s:i4", resultName );
    }

    switch( a.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( e1, "#%d:c1", a.value.c );
        if( resultType == ResultTypeSwitch::SAME_AS_FIRST_OPRAND ) {
            sprintf( result, "%%%s:c1", resultName );
        }
        break;
    case VarType::ExprType::INTEGER:
        sprintf( e1, "#%d:i4", a.value.i );
        if( resultType == ResultTypeSwitch::SAME_AS_FIRST_OPRAND ) {
            sprintf( result, "%%%s:i4", resultName );
        }
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( e1, "#%u:u4", a.value.u );
        if( resultType == ResultTypeSwitch::SAME_AS_FIRST_OPRAND ) {
            sprintf( result, "%%%s:u4", resultName );
        }
        break;
    case VarType::ExprType::REAL:
        sprintf( e1, "#%lf:f8", a.value.d );
        if( resultType == ResultTypeSwitch::SAME_AS_FIRST_OPRAND ) {
            sprintf( result, "%%%s:f8", resultName );
        }
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( e1, "#%s:s%d", a.value.s->c_str(), a.value.s->size() );
        if( resultType == ResultTypeSwitch::SAME_AS_FIRST_OPRAND ) {
            sprintf( result, "%%%s:s%d", resultName, a.value.s->size() );
        }
        break;
    case VarType::ExprType::VAR:
        sprintf( e1, "%%%s:%s", a.pvar->getName().c_str(), a.pvar->getTypeName().c_str() );
        if( resultType == ResultTypeSwitch::SAME_AS_FIRST_OPRAND ) {
            sprintf( result, "%%%s:%s", resultName, a.pvar->getTypeName().c_str() );
        }
        break;
    default:
        ;
    }

    switch( b.type ) {
    case VarType::ExprType::CHARACTER:
        sprintf( e2, "#%d:c1", b.value.c );
        if( resultType == ResultTypeSwitch::SAME_AS_SECOND_OPRAND ) {
            sprintf( result, "%%%s:c1", resultName );
        }
        break;
    case VarType::ExprType::INTEGER:
        sprintf( e2, "#%d:i4", b.value.i );
        if( resultType == ResultTypeSwitch::SAME_AS_SECOND_OPRAND ) {
            sprintf( result, "%%%s:i4", resultName );
        }
        break;
    case VarType::ExprType::U_INTEGER:
        sprintf( e2, "#%u:u4", b.value.u );
        if( resultType == ResultTypeSwitch::SAME_AS_SECOND_OPRAND ) {
            sprintf( result, "%%%s:u4", resultName );
        }
        break;
    case VarType::ExprType::REAL:
        sprintf( e2, "#%lf:f8", b.value.d );
        if( resultType == ResultTypeSwitch::SAME_AS_SECOND_OPRAND ) {
            sprintf( result, "%%%s:f8", resultName );
        }
        break;
    case VarType::ExprType::STRING:     // can not be reached
        sprintf( e2, "#%s:s%d", b.value.s->c_str(), b.value.s->size() );
        if( resultType == ResultTypeSwitch::SAME_AS_SECOND_OPRAND ) {
            sprintf( result, "%%%s:s%d", resultName, b.value.s->size() );
        }
        break;
    case VarType::ExprType::VAR:
        sprintf( e2, "%%%s:%s", b.pvar->getName().c_str(), b.pvar->getTypeName().c_str() );
        if( resultType == ResultTypeSwitch::SAME_AS_SECOND_OPRAND ) {
            sprintf( result, "%%%s:%s", resultName, b.pvar->getTypeName().c_str() );
        }
        break;
    default:
        ;
    }

    fprintf( pfimp, "%s %s, %s, %s\n", op, result, e1, e2 );
    return new string(resultName);
}

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


void CodeGenerator::emitMove( const Variable::Expression &r, const Variable::Expression &a ) {
    char result[Symbol::MaxNameLen];
    char val[Symbol::MaxNameLen];

    sprintf( result, "%%%s:%s", r.pvar->getName().c_str(), r.pvar->getTypeName().c_str() );
    getFullName( a, val );

    fprintf( pfimp, "MOV %s, %s\n", result, val );
}

void CodeGenerator::emitAssignment( const char *op, const Variable::Expression &a, const Variable::Expression &b ) {
    char result[Symbol::MaxNameLen];
    char val[Symbol::MaxNameLen];

    sprintf( result, "%%%s:%s", a.pvar->getName().c_str(), a.pvar->getTypeName().c_str() );
    getFullName( b, val );

    fprintf( pfimp, "%s %s, %s, %s\n", op, result, result, val );
}


void CodeGenerator::emitBranch( const char *op, const char *labelName, const Variable::Expression &c ) {
    char cond[Symbol::MaxNameLen];

    getFullName( c, cond );

    fprintf( pfimp, "%s %s, %s\n", op, labelName, cond );
}


void CodeGenerator::emitJump( const char *labelName ) {
    fprintf( pfimp, "JMP %s\n", labelName );
}


void CodeGenerator::emitLabel( const char* labelName ) {
    fprintf( pfimp, "%s:\n", labelName );
}

std::string* CodeGenerator::emitAndOr( LogicOperation lop, const Variable::Expression &a, const Variable::Expression &b ) {
    char jop[2][4] = { "JCF", "JTF" };
    char result[Symbol::MaxNameLen];
    char resultName[Symbol::MaxNameLen];
    char e1[Symbol::MaxNameLen];
    char e2[Symbol::MaxNameLen];
    
    sprintf( resultName, "@t%d", tempVarCount++ );
    sprintf( result, "%%%s:i4", resultName );

    getFullName( a, e1 );
    getFullName( b, e2 );
    
    fprintf( pfimp, "NE %s, %s, #0:i4\n", result, e1 );
    Label label;
    fprintf( pfimp, "%s %s, %s\n", jop[lop], label.getName().c_str(), result );
    fprintf( pfimp, "NE %s, %s, #0:i4\n", result, e2 );
    emitLabel( label.getName().c_str() );

    return new string(resultName);
}