// syntax parser for target code generation

%{
    #include <cstdio>
    #include <string>
    #include "ir.tab.hpp"
    
    int irlex();
    void irerror(const char *s);

    extern int line;
%}

// -------------------------

%name-prefix "ir"

%union {
    std::string *pstr;
};

// -------------------------

%token MOV ADD SUB MUL DIV MOD INC DEC
%token AND OR XOR NEG LSHL LSHR ASHL ASHR
%token EQ NE GT LT GE LE
%token LEA DREF CALL INVOKE RET JCT JCF JMP
%token <pstr> C1 F8 F4 I8 I4 I2 U8 U4 U2 STR
%token <pstr> USER_VAR TEMP_VAR CONSTANT ID

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

	}
	| C1 {
		// DL
		// ADD/...
	}
	| I8 {
		// EDX, EAX
		// ADD/...
	}
	| I4 {
		// EDX
		// ADD/...
	}
	| I2 {
		// DX
		// ADD/...
	}
	| U8 {

	}
	| U4 {

	}
	| U2 {

	}
	| F8 {
		// FLD ST(0)
		// ST(0)
		// FADD/FDIV/F...
	}
	| F4 {
		// FLD ST(0)
		// ST(0)
		// FADD/...
	}
	;

var:
	USER_VAR ':' type {

	}
	| TEMP_VAR {

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

	}
	| CONSTANT ':' type {

	}
	;

lvalue:
	var ':' type {

	}
	;

func:
	ID
	;

label:
	ID {

	}
	;

stmt:
	block
	| MOV  lvalue ',' rvalue {

	}
	| ADD  lvalue ',' rvalue ',' rvalue {

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
	| JCT  label ',' rvalue {

	}
	| JCF  label ',' rvalue {

	}
	| JMP  label {

	}
	;

// -------------------------



%%	// =================================

void irerror(const char *msg) {
    fprintf( stderr, "Error: %s [line %d]\n", msg, line);
}
