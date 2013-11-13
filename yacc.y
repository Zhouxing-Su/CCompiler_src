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
	#include "CodeGenerator.h"
    #include "Log.h"
    #include "yacc.tab.hpp"

    #define	log(msg)	Log::syntaxAnalasisLog(msg,0)
    
    int yylex();
    void yyerror(const char *s);

    extern int LineCount;
%}

// -------------------------

%union {
    int i;

    std::string *pstr;
    Symbol *symbol;
    SymbolTable *pst;
    std::vector<int> *width;
    struct name {
        int depth;
        std::vector<int> *width;
        std::string *pstr;
		Variable::Expression initExpr;
    } name;
    Variable::Expression expr;
};


%type <pst> argvList
%type <symbol> atomType
%type <width> bracket
%type <symbol> compoundTypeDef
%type <expr> expr
%type <symbol> funcDef
%type <name> name
%type <i> stars
%type <symbol> userDefType
%type <pst> varDecl
%type <pst> varDecls
%type <symbol> varDef
%type <name> varInit
%type <symbol> varType
%type <symbol> ifCond

// -------------------------
%token <pstr> ID
%token <expr> VARIABLE
%token <expr> CONSTANT
%token <symbol> DEFINED_TYPE

%token KW_AUTO KW_CONST KW_EXTERN KW_REGISTER KW_STATIC KW_VOLATILE KW_TYPEDEF
%token KW_CHAR KW_DOUBLE KW_FLOAT KW_INT KW_LONG KW_SHORT KW_SIGNED KW_UNSIGNED KW_VOID
%token KW_STRUCT KW_UNION KW_ENUM
%token KW_BREAK KW_CASE KW_CONTINUE KW_DEFAULT KW_DO KW_ELSE KW_FOR KW_GOTO KW_IF KW_RETURN KW_SWITCH KW_WHILE 

// -------------------------
%destructor {	delete $$;	} <ID>

// -------------------------

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
%nonassoc KW_ELSE

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
        free( $2 );		// do not use 'delete'! or the 'VarType' contained in will be deleted either.
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
    | KW_ENUM ID {	log("enum definition");	}	 '{' enumList '}' {
		// currently ignore this syntax
	}
    ;
varDecls:		// done
    varDecls varDecl ';' {
        $$->appendTable( $2 );
        free( $2 );		// do not use 'delete'! or the 'VarType' contained in will be deleted either.
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
        $$ = $1 + 1;
    }
    | {
        $$ = 0;
    }
    ;

bracket:		// done
    bracket '[' expr ']' {
        $1->push_back( $<i>3 );
        $$ = $1;
    }
    | {
        $$ = new std::vector<int>();
    }
    ;

name:			// done
    stars ID bracket	{
        $$.depth = $1;
        $$.pstr = $2;
        if( $3->size() == 0 ) {
            $$.width = NULL;
            delete $3;
        } else {
            $$.width = $3;
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
        Variable *var = new Variable( *($2.pstr), new VarType( $1->getName(), $2.depth, 
            dynamic_cast<VarType*>($1)->getCompoundLevel(), $2.width, new SymbolTable( 1, $1 ) ) );
        if( var->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
            Log::VariableRedefinitionError( $2.pstr );
            YYABORT;
        } else if( ( $2.initExpr.type != VarType::ExprType::VAR ) || ( $2.initExpr.pvar != NULL ) ) {
			Variable::Expression expr;
			expr.name = NULL;
			expr.type = VarType::ExprType::VAR;
			expr.mutablity = Variable::Mutablity::LVALUE;
			expr.pvar = var;
            if( (Variable::isCompatConv( expr, $2.initExpr ) == true 
				|| Variable::isCompatConv( $2.initExpr, expr ) == true) ) {
				CodeGenerator::cg->emitMove( expr, $2.initExpr );
			} else {
				Log::IncompatibleTypeError();
				YYABORT;
			}
		}
    }
    | varDef ',' varInit {
        $$ = $1;
        Variable *var = new Variable( *($3.pstr), new VarType( $1->getName(), $3.depth, 
            dynamic_cast<VarType*>($1)->getCompoundLevel(), $3.width, new SymbolTable( 1, $1 ) ) );
        if( var->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
			Log::VariableRedefinitionError( $3.pstr );
            YYABORT;
        } else if( ( $3.initExpr.type != VarType::ExprType::VAR ) || ( $3.initExpr.pvar != NULL ) ) {
			Variable::Expression expr;
			expr.name = NULL;
			expr.type = VarType::ExprType::VAR;
			expr.mutablity = Variable::Mutablity::LVALUE;
			expr.pvar = var;
            if( (Variable::isCompatConv( expr, $3.initExpr ) == true 
				|| Variable::isCompatConv( $3.initExpr, expr ) == true) ) {
				CodeGenerator::cg->emitMove( expr, $3.initExpr );
			} else {
				Log::IncompatibleTypeError();
				YYABORT;
			}
		}
    }
    ;
varInit:		// ?done
    name {
        $$ = $1;	// use (type == VAR) but (pvar == NULL) to indicate there is no initialization
		$$.initExpr.type = VarType::ExprType::VAR;
		$$.initExpr.pvar = NULL;
    }
    | name '=' expr {
        $$ = $1;
		$$.initExpr = $3;
    }
    | name '=' '{' expr '}' {	//? it is different from the comma expression
        $$ = $1;
		$$.initExpr = $4;
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

funcImplement:	// undone	// argument should be varDecl
    funcDef {
        Log::syntaxAnalasisLog("function implementation", -1);
    }
    block {
        // pop the (CS) and PC register ( pushed in funcCall )
    }
    ;

block:			// undone
    '{' {
            SymbolTable::enterNewScope();
			CodeGenerator::cg->emitBlock('{');
        }
        stmts '}' {
            SymbolTable::exitCurrentScope();
			CodeGenerator::cg->emitBlock('}');
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
    | KW_RETURN expr ';' {
        log("return statement");
    }
    | KW_RETURN ';' {
        log("return statement");
    }
    | KW_BREAK ';' {
        log("break statement");
    }
    | KW_CONTINUE ';' {
        log("continue statement");
    }
    | KW_GOTO ID ';' {
        log("goto statement");
    }
    | ID ':'	/* label of "goto" */ {	
        log("lable of goto");
        if( ( new Label( *$1 ) )->attach( SymbolTable::getCurrentScope() ) == Symbol::IDstate::CONFLICT ) {
            Log::ConflictLabelNameError( *$1 );
            YYABORT;
        }
    }
    | error ';'
    | error '}'
    ;

expr:			// undone
    // either left or right value expression
    VARIABLE {
        $$ = $1;
    }
    | expr '[' expr ']'
    | '*' expr %prec DEREF {
		
	}
    | expr ARROW expr
    | expr '.' expr
    | '(' expr ')' {
		$$ = $2;
	}
    // left value expression
    | expr '=' expr {		// done
		if( $1.mutablity == Variable::Mutablity::LVALUE
			&& (Variable::isCompatConv( $1, $3 ) == true 
				|| Variable::isCompatConv( $3, $1 ) == true) ) {
			CodeGenerator::cg->emitMove( $1, $3 );
			$$ = $1;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
		}
	}
    | expr ADDASS expr {	// done
		if( $1.mutablity == Variable::Mutablity::LVALUE
			&& (Variable::isCompatConv( $1, $3 ) == true 
				|| Variable::isCompatConv( $3, $1 ) == true) ) {
			CodeGenerator::cg->emitAssignment( "ADD", $1, $3 );
			$$ = $1;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
		}
	}
    | expr SUBASS expr {	// done
		if( $1.mutablity == Variable::Mutablity::LVALUE
			&& (Variable::isCompatConv( $1, $3 ) == true 
				|| Variable::isCompatConv( $3, $1 ) == true) ) {
			CodeGenerator::cg->emitAssignment( "SUB", $1, $3 );
			$$ = $1;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
		}
	}
    | expr MULASS expr {	// done
		if( $1.mutablity == Variable::Mutablity::LVALUE
			&& (Variable::isCompatConv( $1, $3 ) == true 
				|| Variable::isCompatConv( $3, $1 ) == true) ) {
			CodeGenerator::cg->emitAssignment( "MUL", $1, $3 );
			$$ = $1;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
		}
	}
    | expr DIVASS expr {	// done
		if( $1.mutablity == Variable::Mutablity::LVALUE
			&& (Variable::isCompatConv( $1, $3 ) == true 
				|| Variable::isCompatConv( $3, $1 ) == true) ) {
			CodeGenerator::cg->emitAssignment( "DIV", $1, $3 );
			$$ = $1;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
		}
	}
    | expr MODASS expr
    | expr ANDASS expr
    | expr XORASS expr
    | expr ORASS expr
    | expr SHLASS expr
    | expr SHRASS expr
    | INC expr {
		
	}
    | DEC expr {
		
	}
    // right value expression
    | CONSTANT {
        $$ = $1;
    }
    | SIZEOF expr {
		
	}
    | SIZEOF '(' varType ')' {
		
	}
    | funcCall {
		
	}
    | '&' expr %prec ADDROF {
		
	}
    | expr '?' expr ':' expr
    | expr ',' expr
    | expr OR expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitAndOr( CodeGenerator::LogicOperation::OR, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
			$$.type = VarType::ExprType::VAR;
			$$.mutablity = Variable::Mutablity::RVALUE;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
	}
    | expr AND expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitAndOr( CodeGenerator::LogicOperation::AND, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
			$$.type = VarType::ExprType::VAR;
			$$.mutablity = Variable::Mutablity::RVALUE;
		} else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
	}
    | expr '|' expr
    | expr '^' expr
    | expr '&' expr
    | expr EQ expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "EQ", CodeGenerator::ResultTypeSwitch::BOOL_VALUE, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr NEQ expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "NE", CodeGenerator::ResultTypeSwitch::BOOL_VALUE, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr '<' expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "LT", CodeGenerator::ResultTypeSwitch::BOOL_VALUE, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr LE expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "LE", CodeGenerator::ResultTypeSwitch::BOOL_VALUE, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr '>' expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "GT", CodeGenerator::ResultTypeSwitch::BOOL_VALUE, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr GE expr {	// done
		if( Variable::isCompatConv( $1, $3 ) == true || Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "GE", CodeGenerator::ResultTypeSwitch::BOOL_VALUE, $1, $3 );
			$$.pvar = new Variable( *($$.name), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr SHL expr
    | expr SHR expr
    | expr '+' expr {	// done
        if( Variable::isCompatConv( $1, $3 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "ADD", CodeGenerator::ResultTypeSwitch::SAME_AS_SECOND_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $3.pvar->getType() ) );
        } else if ( Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "ADD", CodeGenerator::ResultTypeSwitch::SAME_AS_FIRST_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $1.pvar->getType() ) );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
    }
    | expr '-' expr {	// done
        if( Variable::isCompatConv( $1, $3 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "SUB", CodeGenerator::ResultTypeSwitch::SAME_AS_SECOND_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $3.pvar->getType() ) );
        } else if ( Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "SUB", CodeGenerator::ResultTypeSwitch::SAME_AS_FIRST_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $1.pvar->getType() ) );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
    }
    | expr '*' expr {	// done
        if( Variable::isCompatConv( $1, $3 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "MUL", CodeGenerator::ResultTypeSwitch::SAME_AS_SECOND_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $3.pvar->getType() ) );
        } else if ( Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "MUL", CodeGenerator::ResultTypeSwitch::SAME_AS_FIRST_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $1.pvar->getType() ) );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
    }
    | expr '/' expr {	// done
        if( Variable::isCompatConv( $1, $3 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "DIV", CodeGenerator::ResultTypeSwitch::SAME_AS_SECOND_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $3.pvar->getType() ) );
        } else if ( Variable::isCompatConv( $3, $1 ) == true ) {
			$$.name = CodeGenerator::cg->emitExpression( "DIV", CodeGenerator::ResultTypeSwitch::SAME_AS_FIRST_OPRAND, $1, $3 );
			$$.pvar = new Variable( *($$.name), new VarType( $1.pvar->getType() ) );
        } else {
            Log::IncompatibleTypeError();
            YYABORT;
        }
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
    }
    | expr '%' expr {	// undone
		// the two operands should both be integer value
	}
    | '!' expr {
		
	}
    | '~' expr {
		
	}
    | expr INC {	// done
		if( $1.mutablity == Variable::Mutablity::LVALUE ) {
			$$.name = CodeGenerator::cg->emitInc( "INC", $1 );
			if( $$.name != NULL ) {
				$$.pvar = new Variable( *($$.name), new VarType( $1.pvar->getType() ) );
			} else {
				Log::IncompatibleTypeError();
				YYABORT;
			}
		} else {
			Log::AssignToRightValueError();
            YYABORT;
		}
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | expr DEC {	// done
		if( $1.mutablity == Variable::Mutablity::LVALUE ) {
			$$.name = CodeGenerator::cg->emitInc( "DEC", $1 );
			if( $$.name != NULL ) {
				$$.pvar = new Variable( *($$.name), new VarType( $1.pvar->getType() ) );
			} else {
				Log::IncompatibleTypeError();
				YYABORT;
			}
		} else {
			Log::AssignToRightValueError();
            YYABORT;
		}
        $$.type = VarType::ExprType::VAR;
        $$.mutablity = Variable::Mutablity::RVALUE;
	}
    | '+' expr %prec POS {
		
	}
    | '-' expr %prec NEG {
		
	}
    ;



funcCall:		// undone
    ID '(' expr ')' {
        log("function call");
        // push the (CS) and PC register
    }
    ;

ifCond:			// done
	KW_IF '(' {
			CodeGenerator::cg->emitBlock('{');
		} expr {
			log("if clause");
			$$ = new Label();
			CodeGenerator::cg->emitBranch( "JCF", $$->getName().c_str(), $4 );
		}
	;

if:				// done
    ifCond ')' stmt KW_ELSE {
	  		log("else clause");
			$<symbol>3 = new Label();
			CodeGenerator::cg->emitJump( $<symbol>3->getName().c_str() );
			CodeGenerator::cg->emitLabel( $1->getName().c_str() );
		}
		stmt {
			CodeGenerator::cg->emitLabel( $<symbol>3->getName().c_str() );
			CodeGenerator::cg->emitBlock('}');
		} 
    | ifCond ')' stmt {
		CodeGenerator::cg->emitLabel( $1->getName().c_str() );
		CodeGenerator::cg->emitBlock('}');
	} 
    ;
    
while:			// done
    KW_WHILE {
            log("while clause");
            $<symbol>1 = new Label();	// while block start point
			CodeGenerator::cg->emitBlock('{');
            CodeGenerator::cg->emitLabel( $<symbol>1->getName().c_str() );
        }
        '(' expr ')' {
			$<symbol>3 = new Label();	// while block end point
			CodeGenerator::cg->emitBranch( "JCF", $<symbol>3->getName().c_str(), $4 );
		}
		stmt {
			CodeGenerator::cg->emitJump( $<symbol>1->getName().c_str() );
            CodeGenerator::cg->emitLabel( $<symbol>3->getName().c_str() );
			CodeGenerator::cg->emitBlock('}');
        }
    ;

dowhile:		// undone
    KW_DO {
            log("do-while clause");
			CodeGenerator::cg->emitBlock('{');
            $<symbol>1 = new Label();
            CodeGenerator::cg->emitLabel( $<symbol>1->getName().c_str() );
        }
        stmt KW_WHILE '(' expr ')' ';'	 {
			CodeGenerator::cg->emitBlock('}');
		}
    ;

for:			// undone
    KW_FOR {
			CodeGenerator::cg->emitBlock('{');
		} '(' forExpr ';' {
            log("for clause");
            $<symbol>1 = new Label();
        }
        forExpr ';' forExpr ')' stmt {
			CodeGenerator::cg->emitBlock('}');
		}
    ;
forExpr:		// undone
      expr
    | 
    ;
    
switch:			// undone
      KW_SWITCH {
				CodeGenerator::cg->emitBlock('{');
			} '(' expr ')' {
				log("switch clause");
			} '{' cases defaultCase '}' {
				CodeGenerator::cg->emitBlock('}');
			}
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
