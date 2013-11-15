// syntax parser for target code generation

%{
    #include <cstdio>
    #include <string>
	#include "CodeGenerator.h"
    #include "ir.tab.hpp"
    
    int irlex();
    void irerror(const char *s);

    extern int line;
%}

// -------------------------

%name-prefix "ir"

%union {
    std::string *pstr;
	struct {
		std::string *pname;
		CodeGenerator::MemTypeIndex mtype;
		CodeGenerator::RegTypeIndex rtype;
	} value;
};

// -------------------------

%token MOV ADD SUB MUL DIV MOD INC DEC
%token AND OR XOR NEG LSHL LSHR ASHL ASHR
%token EQ NE GT LT GE LE
%token LEA DREF CALL INVOKE RET JCT JCF JMP
%token <pstr> C1 F8 F4 I8 I4 I2 U8 U4 U2 STR
%token <pstr> USER_VAR TEMP_VAR CONSTANT ID

%type <value> type
%type <value> lvalue
%type <value> rvalue

// -------------------------
%destructor {	delete $$;	} <ID>

// -------------------------
%start program

%%	// =================================

program:
    stmts
    ;

stmts:
	stmts stmt
	|
	;

block:
	'{' {
			
		} stmts '}' {
			
		}
	;

type:
	STR {
		$$.mtype = CodeGenerator::MemTypeIndex::STR;
		$$.rtype = CodeGenerator::RegTypeIndex::ESI;
		$$.pname = NULL;
	}
	| C1 {
		$$.mtype = CodeGenerator::MemTypeIndex::C1;
		$$.rtype = CodeGenerator::RegTypeIndex::DL;
		$$.pname = NULL;
	}
	| I8 {
		$$.mtype = CodeGenerator::MemTypeIndex::I8;
		$$.rtype = CodeGenerator::RegTypeIndex::EDXEAX;
		$$.pname = NULL;
	}
	| I4 {
		$$.mtype = CodeGenerator::MemTypeIndex::I4;
		$$.rtype = CodeGenerator::RegTypeIndex::EDX;
		$$.pname = NULL;
	}
	| I2 {
		$$.mtype = CodeGenerator::MemTypeIndex::I2;
		$$.rtype = CodeGenerator::RegTypeIndex::DX;
		$$.pname = NULL;
	}
	| U8 {
		$$.mtype = CodeGenerator::MemTypeIndex::U8;
		$$.rtype = CodeGenerator::RegTypeIndex::EDXEAX;
		$$.pname = NULL;
	}
	| U4 {
		$$.mtype = CodeGenerator::MemTypeIndex::U4;
		$$.rtype = CodeGenerator::RegTypeIndex::EDX;
		$$.pname = NULL;
	}
	| U2 {
		$$.mtype = CodeGenerator::MemTypeIndex::U2;
		$$.rtype = CodeGenerator::RegTypeIndex::DL;
		$$.pname = NULL;
	}
	| F8 {
		$$.mtype = CodeGenerator::MemTypeIndex::F8;
		$$.rtype = CodeGenerator::RegTypeIndex::ST;
		$$.pname = NULL;
	}
	| F4 {
		$$.mtype = CodeGenerator::MemTypeIndex::F4;
		$$.rtype = CodeGenerator::RegTypeIndex::ST;
		$$.pname = NULL;
	}
	;

argList:
	argList ',' rvalue {

	}
	| rvalue {

	}
	;

rvalue:
	lvalue {
		$$ = $1;
	}
	| CONSTANT ':' type {
		$$.pname = $1;
		$$.mtype = $3.mtype;
		$$.rtype = $3.rtype;
	}
	;

lvalue:
	USER_VAR ':' type {
		$$.pname = $1;
		$$.mtype = $3.mtype;
		$$.rtype = $3.rtype;
	}
	| TEMP_VAR ':' type {
		$$.pname = $1;
		$$.mtype = $3.mtype;
		$$.rtype = $3.rtype;
	}
	;

func:
	ID {

	}
	;

label:
	ID ':' {

	}
	;

stmt:
	block
	| label
	| MOV  lvalue ',' rvalue {
		CodeGenerator::cg->emitLoad_a( $2.mtype, $4.mtype, $4.pname );
		CodeGenerator::cg->emitWrite_r( $2.mtype, $2.rtype, $2.pname );
	}
	| ADD  lvalue ',' rvalue ',' rvalue {
		CodeGenerator::cg->emitLoad_a( $2.mtype, $4.mtype, $4.pname );
		CodeGenerator::cg->emitOperation();
		CodeGenerator::cg->emitWrite_r( $2.mtype, (($4.rtype>$2.rtype)?($4.rtype):($2.rtype)), $2.pname );
	}
	| SUB  lvalue ',' rvalue ',' rvalue {

	}
	| MUL  lvalue ',' rvalue ',' rvalue {

	}
	| DIV  lvalue ',' rvalue ',' rvalue {

	}
	| MOD  lvalue ',' rvalue ',' rvalue {

	}
	| INC  lvalue {

	}
	| DEC  lvalue {

	}
	| AND  lvalue ',' rvalue ',' rvalue {

	}
	| OR   lvalue ',' rvalue ',' rvalue {

	}
	| XOR  lvalue ',' rvalue ',' rvalue {

	}
	| NEG  lvalue ',' rvalue {

	}
	| LSHL lvalue ',' rvalue ',' rvalue {

	}
	| LSHR lvalue ',' rvalue ',' rvalue {

	}
	| ASHL lvalue ',' rvalue ',' rvalue {

	}
	| ASHR lvalue ',' rvalue ',' rvalue {

	}
	| EQ   lvalue ',' rvalue ',' rvalue {

	}
	| NE   lvalue ',' rvalue ',' rvalue {

	}
	| GT   lvalue ',' rvalue ',' rvalue {

	}
	| LT   lvalue ',' rvalue ',' rvalue {

	}
	| GE   lvalue ',' rvalue ',' rvalue {

	}
	| LE   lvalue ',' rvalue ',' rvalue {

	}
	| LEA  lvalue ',' rvalue {

	}
	| DREF lvalue ',' rvalue {

	}
	| CALL lvalue ',' func ',' argList {

	}
	| INVOKE func ',' argList {

	}
	| RET {

	}
	| RET  rvalue {

	}
	| JCT  ID ',' rvalue {

	}
	| JCF  ID ',' rvalue {

	}
	| JMP  ID {

	}
	;

// -------------------------



%%	// =================================

void irerror(const char *msg) {
    fprintf( stderr, "irparse Error: %s [line %d]\n", msg, line);
}
