/*
int switch clause, constant after "case" can only be constant parsed by lex 
    and regard constant expression as error, 
    but think real number and string constant correct .
varinit can be extend to fit the initialization of array and struct .
do not support explicit type cast like "int i = (int)c;" in the "expr" production .
the error "void a" in "void f(void a)" may not be recognize .
do not support type modifiers such as "const" .
"name" needs to be modified to fit the rule that "int a[][3]" is right and "int a[3][]" is wrong .
*/

%{
    #include <cstdio>
    #include <string>
    #include "SymbolTable.h"
    #include "FileWraper.h"
    #include "Log.h"
    #include "yacc.tab.hpp"

    #define	log(msg)	Log::syntaxAnalasisLog(msg,0)
    
    int yylex();
    void yyerror(const char *s);

    extern int LineCount;
%}

// -------------------------

%union {
    char c;
    int i;
    unsigned u;
    long l;
    short s;
    float f;
    double d;

    std::string *pstr;
    Symbol *symbol;
    SymbolTable *pst;
    std::vector<int> *width;
    struct name {
        int depth;
        std::vector<int> *width;
        std::string *pstr;
    } name;
};

%type <pst> argvList
%type <symbol> atomType
%type <symbol> compoundTypeDef
%type <symbol> funcDef
%type <name> name
%type <symbol> userDefType
%type <pst> varDecl
%type <pst> varDecls
%type <symbol> varDef
%type <name> varInit
%type <symbol> varType

// -------------------------
%token <pstr> ID
%token CONSTANT
%token <symbol> DEFINED_TYPE

%token KW_AUTO KW_CONST KW_EXTERN KW_REGISTER KW_STATIC KW_VOLATILE KW_TYPEDEF
%token KW_CHAR KW_DOUBLE KW_FLOAT KW_INT KW_LONG KW_SHORT KW_SIGNED KW_UNSIGNED KW_VOID
%token KW_STRUCT KW_UNION KW_ENUM
%token KW_BREAK KW_CASE KW_CONTINUE KW_DEFAULT KW_DO KW_ELSE KW_FOR KW_GOTO KW_IF KW_RETURN KW_SWITCH KW_WHILE 

// -------------------------
%destructor {	delete $$;	} <ID>

// -------------------------

%right IFX KW_ELSE
%left	','
%right	'=' ADDASS SUBASS MULASS DIVASS MODASS ANDASS XORASS ORASS SHLASS SHRASS
%right	'?' ':'
%left	OR
%left	AND
%left	'|'
%left	'^'
%left	'&'
%left	EQ NEQ 		// "==" "!="
%left	'<' LE '>' GE 	// '<' "<=" '>' ">="
%left	SHL SHR		// "<<" ">>"
%left	'+' '-'
%left	'*' '/' '%'
%right	'!' '~' INC DEC POS NEG DEREF ADDROF SIZEOF		// '!' '~' "++" "--" '+' '-' '*' '&' "sizeof" //
%left	ARROW '.'	'[' ']' '(' ')'	// "->"

%start program

%%	// =================================

program:
      program	typeDefineStmt
    | program	varDef ';'		{	log("global variable definition");	}
    | program	varDeclStmt
    | program	funcDecl
    | program	funcImplement
    |
    | error ';'
    | error '}'
    ;

// -------------------------

typeDefineStmt:	// done
      typeDefine ';'
    ;
typeDefine:		// done
    KW_TYPEDEF varDecl {
        log("user type definition");
        for( int i = 0 ; i < $2->size() ; ++i ) {
            (*$2)[i]->attach( SymbolTable::getCurrentScope() );
        }
        delete $2;
    }
    | compoundTypeDef {
        // nothing to do
    }
    ;
compoundTypeDef:	// undone
    KW_STRUCT ID {	log("struct definition");	}	'{' varDecls '}' {
        $$ = new VarType( *($2), 0, VarType::CompoundLevel::STRUCT, NULL, $5 );
        $$->attach( SymbolTable::getCurrentScope() );
    }
    | KW_UNION ID {	log("union definition");	}	'{' varDecls '}' {
        $$ = new VarType( *($2), 0, VarType::CompoundLevel::UNION, NULL, $5 );
        $$->attach( SymbolTable::getCurrentScope() );
    }
    | KW_ENUM ID {	log("enum definition");	}	 '{' enumList '}'
    ;
varDecls:		// done
    varDecls varDecl ';' {
        $$->appendTable( $2 );
        delete $2;
    }
    | varDecl ';' {
        $$ = $1;
    }
    ;
enumList:		// undone
      ID
    | enumList ',' ID
    ;
    
// -------------------------

stars:			// done
    stars '*' {
        $<i>$ = $<i>1 + 1;
    }
    | {
        $<i>$ = 0;
    }
    ;

bracket:		// done
    bracket '[' expr ']' {
        $<width>1->push_back( $<i>3 );
        $<width>$ = $<width>1;
    }
    | {
        $<width>$ = new std::vector<int>();
    }
    ;

name:			// done
    stars ID bracket	{
        $$.depth = $<i>1;
        $$.pstr = $2;
        if( $<width>3->size() == 0 ) {
            $$.width = NULL;
            delete $<width>3;
        } else {
            $$.width = $<width>3;
        }
    }
    ;

varDeclStmt:	// undone
    KW_EXTERN varDecl ';' {
        log("global variable declaration");
        delete $2;
    }
    ;
varDecl:		// done
    varType name {
        $$ = new SymbolTable();
        $$->addSymbol( new VarType( *($2.pstr), $2.depth, dynamic_cast<VarType*>($1)->getCompoundLevel(),
            $2.width, new SymbolTable( 1, $1 ) ) );
    }
    | varDecl ',' name {
        $1->addSymbol( new VarType( *($3.pstr), $3.depth, dynamic_cast<VarType*>( (*$$)[0] )->getCompoundLevel(),
            $3.width, dynamic_cast<VarType*>( (*$$)[0] )->getMemberList() ) );
        $$ = $1;
    }
    ;
varDef:			// done
    varType varInit {
        $$ = $1;
        Variable *var = new Variable( *($2.pstr), new VarType( *($2.pstr), $2.depth, 
            dynamic_cast<VarType*>($1)->getCompoundLevel(), $2.width, new SymbolTable( 1, $1 ) ) );
        if( var->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
            Log::VariableRedefinitionError( $2.pstr );
            YYABORT;
        }
    }
    | varDef ',' varInit {
        $$ = $1;
        Variable *var = new Variable( *($3.pstr), new VarType( *($3.pstr), $3.depth, 
            dynamic_cast<VarType*>($1)->getCompoundLevel(), $3.width, new SymbolTable( 1, $1 ) ) );
        if( var->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
            Log::VariableRedefinitionError( $3.pstr );
            YYABORT;
        }
    }
    ;
varInit:		// undone
    name {
        $$ = $1;
    }
    | name '=' expr {
        $$ = $1;
    }
    | name '=' '{' expr '}' {
        $$ = $1;
    }
    ;
varType:		// done
    atomType {
        $$ = $1;
    }
    | userDefType {
        $$ = $1;
    }
    ;
userDefType:	// done
    compoundTypeDef {
        $$ = $1;
    }
    | KW_STRUCT DEFINED_TYPE {
        if( dynamic_cast<VarType*>($2)->getCompoundLevel() == VarType::CompoundLevel::STRUCT ) {
            $$ = $2;
        } else {
            Log::CompoundLevelConflictError( $2->getName() , "struct" );
            YYABORT;
        }
    }
    | KW_UNION DEFINED_TYPE {
        if( dynamic_cast<VarType*>($2)->getCompoundLevel() == VarType::CompoundLevel::UNION ) {
            $$ = $2;
        } else {
            Log::CompoundLevelConflictError( $2->getName() , "union" );
            YYABORT;
        }
    }
    | KW_ENUM DEFINED_TYPE {
        if( dynamic_cast<VarType*>($2)->getCompoundLevel() == VarType::CompoundLevel::ENUM ) {
            $$ = $2;
        } else {
            Log::CompoundLevelConflictError( $2->getName() , "enum" );
            YYABORT;
        }
    }
    | DEFINED_TYPE {	// leave out 'struct'/'union'/'enum' is always ok.
        $$ = $1;
    }
    ;
atomType:		// done
    KW_CHAR {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::CHAR];
    }
    | KW_INT {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::INT];
    }
    | KW_LONG {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::LONG];
    }
    | KW_SHORT {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::SHORT];
    }
    | KW_FLOAT {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::FLOAT];
    }
    | KW_DOUBLE {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::DOUBLE];
    }
    | KW_SIGNED {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::SIGNED];
    }
    | KW_UNSIGNED {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::UNSIGNED];
    }
    | KW_VOID {
        $$ = VarType::atomTypes[VarType::AtomTypesIndex::VOID];
    }
    ;

// -------------------------

funcDecl:		// nothing to be done
    funcDef ';'	{
        log("function declaration");
    }
    ;

funcDef:		// done
    varType name '(' argvList ')' {
        $$ = new Function( *($2.pstr), new VarType( "ret_" + *($2.pstr), $2.depth, 
            dynamic_cast<VarType*>($1)->getCompoundLevel(), $2.width, new SymbolTable( 1, $1 ) ), $4 );
        if( $$->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
            Log::ConflictPrototypesError( $2.pstr );
            YYABORT;
        }
    }
    ;

argvList:		// done		// used in function definitions
    varType name {
        $$ = new SymbolTable();
        $$->addSymbol( new VarType( *($2.pstr), $2.depth, dynamic_cast<VarType*>($1)->getCompoundLevel(),
            $2.width, new SymbolTable( 1, $1 ) ) );
    }
    | argvList ',' varType name {
        $1->addSymbol( new VarType( *($4.pstr), $4.depth, dynamic_cast<VarType*>($3)->getCompoundLevel(),
            $4.width, new SymbolTable( 1, $3 ) ) );
        $$ = $1;
    }
    | KW_VOID {
        $$ = NULL;
    }
    | {
        $$ = NULL;
    }
    ;

// -------------------------

funcImplement:	// undone
      funcDef {	Log::syntaxAnalasisLog("function implementation", -1);	}	 block
    ;

block:			// undone
    '{' {
            SymbolTable::enterNewScope();
        }
        stmts '}' {
            SymbolTable::exitCurrentScope();
        }
    ;

stmts:			// undone
      stmts stmt
    |
    ;

stmt:			// undone
      block
    | expr ';'			{	log("expression");	}
    | typeDefineStmt
    | varDeclStmt
    | varDef ';'		{	log("local variable definition");	}
    | for
    | while
    | dowhile
    | if
    | switch
    | ';'					{	log("empty statement");	}	
    | KW_RETURN expr ';'		{	log("return statement");	}
    | KW_RETURN ';'				{	log("return statement");	}
    | KW_BREAK ';'					{	log("break statement");	}
    | KW_CONTINUE ';'			{	log("continue statement");	}
    | KW_GOTO ID ';'				{	log("goto statement");	}
    | ID ':'	/* label of "goto" */ {	
        log("lable of goto");
        if( ( new Label( *$1, 0 ) )->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
            Log::ConflictLabelNameError( *$1 );
            YYABORT;
        }
    }
    | error ';'
    | error '}'
    ;

expr:			// undone
    // either left or right value expression
      ID
    | expr '[' expr ']'
    | '*' expr %prec DEREF
    | expr ARROW expr
    | expr '.' expr
    | '(' expr ')'
    // left value expression
    | expr '=' expr
    | expr ADDASS expr
    | expr SUBASS expr
    | expr MULASS expr
    | expr DIVASS expr
    | expr MODASS expr
    | expr ANDASS expr
    | expr XORASS expr
    | expr ORASS expr
    | expr SHLASS expr
    | expr SHRASS expr
    | INC expr
    | DEC expr
    // right value expression
    | CONSTANT
    | SIZEOF expr
    | SIZEOF '(' varType ')'
    | funcCall
    | '&' expr %prec ADDROF
    | expr '?' expr ':' expr
    | expr ',' expr
    | expr OR expr
    | expr AND expr
    | expr '|' expr
    | expr '^' expr
    | expr '&' expr
    | expr EQ expr
    | expr NEQ expr
    | expr '<' expr
    | expr LE expr
    | expr '>' expr
    | expr GE expr
    | expr SHL expr
    | expr SHR expr
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | '!' expr
    | '~' expr
    | expr INC
    | expr DEC
    | '+' expr %prec POS
    | '-' expr %prec NEG
    ;



funcCall:		// undone
      ID '(' expr ')' {	log("function call");	}	
    ;

if:				// undone
      KW_IF '(' expr ')' stmt KW_ELSE stmt	{	log("if-else clause");	} 
    | KW_IF '(' expr ')' stmt %prec IFX {	log("if clause");	} 
    ;
    
while:			// undone
    KW_WHILE {
            log("while clause");
            Label *pl = new Label( 0 );
            if( pl->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
                Log::ConflictLabelNameError( pl->getName() );
                YYABORT;
            }
        }
        '(' expr ')' stmt {
            // set addr and attach all labels created in $5 to symbol table
        }
    ;

dowhile:		// undone
    KW_DO {
            log("do-while clause");
            Label *pl = new Label( 0 );
            if( pl->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
                Log::ConflictLabelNameError( pl->getName() );
                YYABORT;
            }
        }
        stmt KW_WHILE '(' expr ')' ';'	 
    ;

for:			// undone
    KW_FOR '(' forExpr ';' {
            log("for clause");
            Label *pl = new Label( 0 );
            if( pl->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
                Log::ConflictLabelNameError( pl->getName() );
                YYABORT;
            }
        }
        forExpr ';' forExpr ')' stmt
    ;
forExpr:		// undone
      expr
    | 
    ;
    
switch:			// undone
      KW_SWITCH '(' expr ')' {	log("switch clause");	}	'{' cases defaultCase '}'
    ;
cases:			// undone
      case
    | case cases
    ;
case:			// undone
      KW_CASE CONSTANT ':' {	log("case in switch");	}	  stmts
    ;
defaultCase:	// undone
      KW_DEFAULT ':' {	log("default case in switch");	}	  stmts
    |
    ;

/*
argv:			// used in function calls
      expr
    | argv ',' expr		// replaced by expr with comma
    ;
*/

%%	// =================================

void yyerror(const char *msg) {
    fprintf( stderr, "Error: %s (%s[line %d])\n", msg, FileWraper::getPath(), LineCount);
}
