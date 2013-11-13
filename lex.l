/*
do not recognize enum value as constant
*/

%option never-interactive
%option nounistd

digit			[0-9]
letter		[_A-Za-z]

octconst        (0)([0-7]+)
hexconst        (0x)([a-fA-F0-9]+)
intconst		{digit}+
floatconst      ({digit}*"."{digit}+)|({digit}+"."{digit}*)
stringconst     (\"([^"\\\n]|(\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+)))*\"[ \t\v\n\f]*)+
charconst		"'"([^'\\\n]|(\\(['\"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+)))+"'"

PreprocessStatment		^#.*
word			{letter}({letter}|{digit})*
singleCharToken		({singleCharOperator}|{singleCharDelimiter})

singleCharOperator	[~!%^&*\-+=<>/()\,\[\]]
singleCharDelimiter		[\?\:\{\}\;]


%x BLOCKCOMMENT

%{
	#include <cstdlib>
	#include <cstring>
	#include "FileWraper.h"
	#include "SymbolTable.h"
	#include "yacc.tab.hpp"
	#include "Log.h"

	using namespace std;
	
	extern "C" {
		int yywrap(void);
	}

	int LineCount = 1;
	static string textbuf;
%}

%%

"//".*	{	;	}

"/*" {	BEGIN BLOCKCOMMENT;	}
<BLOCKCOMMENT>"*/" {	BEGIN 0;	}
<BLOCKCOMMENT>. {	;	}
<BLOCKCOMMENT>\n {	++LineCount;	}




{PreprocessStatment}	{	;/* return PREP_STMT; */	}


{octconst}    {
    yylval.expr.name = NULL;
    yylval.expr.type = VarType::ExprType::INTEGER;
    yylval.expr.mutablity = Variable::Mutablity::RVALUE;
    yylval.expr.pvar = new Variable( std::string(), VarType::atomTypes[VarType::AtomTypesIndex::UNSIGNED] );
    sscanf( yytext, "%o", &yylval.expr.value.i );
    return CONSTANT;
}
{hexconst}    {
    yylval.expr.name = NULL;
    yylval.expr.type = VarType::ExprType::INTEGER;
    yylval.expr.mutablity = Variable::Mutablity::RVALUE;
    yylval.expr.pvar = new Variable( std::string(), VarType::atomTypes[VarType::AtomTypesIndex::UNSIGNED] );
    sscanf( yytext, "%h", &yylval.expr.value.i );
    return CONSTANT;
}
{intconst}    {
    yylval.expr.name = NULL;
    yylval.expr.type = VarType::ExprType::INTEGER;
    yylval.expr.mutablity = Variable::Mutablity::RVALUE;
    yylval.expr.pvar = new Variable( std::string(), VarType::atomTypes[VarType::AtomTypesIndex::INT] );
    sscanf( yytext, "%i", &yylval.expr.value.i );
    return CONSTANT;
}
{floatconst}    {
    yylval.expr.name = NULL;
    yylval.expr.type = VarType::ExprType::REAL;
    yylval.expr.mutablity = Variable::Mutablity::RVALUE;
    yylval.expr.pvar = new Variable( std::string(), VarType::atomTypes[VarType::AtomTypesIndex::DOUBLE] );
    sscanf( yytext, "%lf", &yylval.expr.value.d );
    return CONSTANT;
}
{stringconst}    {
    yylval.expr.name = NULL;
    yylval.expr.type = VarType::ExprType::STRING;
    yylval.expr.mutablity = Variable::Mutablity::RVALUE;
    yylval.expr.pvar = NULL;
    yylval.expr.value.s = new std::string( yytext );
    return CONSTANT;
}
{charconst}    {
    yylval.expr.name = NULL;
    yylval.expr.type = VarType::ExprType::CHARACTER;
    yylval.expr.mutablity = Variable::Mutablity::RVALUE;
    yylval.expr.pvar = new Variable( std::string(), VarType::atomTypes[VarType::AtomTypesIndex::CHAR] );
    yylval.expr.value.c = *yytext;
    return CONSTANT;
}




{singleCharToken}	{	return *yytext;	}
"<="	{	return LE;	}
">="	{	return GE;	}
"=="	{	return EQ;	}
"!="	{	return NEQ;	}
"++"	{	return INC;	}
"--"	{	return DEC;	}
"<<"	{	return SHL;	}
">>"	{	return SHR;	}
"<<="	{	return SHLASS;	}
">>="	{	return SHRASS;	}
"+="	{	return ADDASS;	}
"-="	{	return SUBASS;	}
"*="	{	return MULASS;	}
"/="	{	return DIVASS;	}
"%="	{	return MODASS;	}
"&="	{	return ANDASS;	}
"|="	{	return ORASS;	}
"^="	{	return XORASS;	}
"&&"	{	return AND;	}
"||"	{	return OR;	}
"sizeof"	{	return SIZEOF;	}





"auto"	{	return KW_AUTO;	}
"const"	{	return KW_CONST;	}
"extern"	{	return KW_EXTERN;	}
"register"	{	return KW_REGISTER;	}
"static"	{	return KW_STATIC;	}
"volatile"	{	return KW_VOLATILE;	}
"typedef"	{	return KW_TYPEDEF;	}

"enum"	{	return KW_ENUM;	}
"struct"	{	return KW_STRUCT;	}
"union"	{	return KW_UNION;	}

"char"	{	return KW_CHAR;	}
"double"	{	return KW_DOUBLE;	}
"float"	{	return KW_FLOAT;	}
"int"	{	return KW_INT;	}
"long"	{	return KW_LONG;	}
"short"	{	return KW_SHORT;	}
"signed"	{	return KW_SIGNED;	}
"unsigned"	{	return KW_UNSIGNED;	}
"void"	{	return KW_VOID;	}

"break"	{	return KW_BREAK;	}
"case"	{	return KW_CASE;	}
"continue"	{	return KW_CONTINUE;	}
"default"	{	return KW_DEFAULT;	}
"do"	{	return KW_DO;	}
"else"	{	return KW_ELSE;	}
"for"	{	return KW_FOR;	}
"goto"	{	return KW_GOTO;	}
"if"	{	return KW_IF;	}
"return"	{	return KW_RETURN;	}
"switch"	{	return KW_SWITCH;	}
"while"	{	return KW_WHILE;	}



{word}	{
	if( ( yylval.symbol = VarType::find( string(yytext) ) ) != NULL ) {
		return DEFINED_TYPE;
	} else if ( ( yylval.expr.pvar = Variable::find( string(yytext) ) ) != NULL ) {
        yylval.expr.name = NULL;
        yylval.expr.type = VarType::ExprType::VAR;
        yylval.expr.mutablity = Variable::Mutablity::LVALUE;
        return VARIABLE;
    } else {
		textbuf = yytext;
		yylval.pstr = new string( textbuf );
		return ID;
	}
}




\n 	{	++LineCount;	}
[ \t\r]	{	;	}

.	{	Log::InvalidCharacherError( yytext );	}

%%

int yywrap(void)
{
	char const *path = FileWraper::nextPath();
	if ( NULL == path ) {
		return 1;
	} else {
		if ( NULL == freopen( path, "r", stdin ) ) {
			Log::FileNotFoundError( FileWraper::getPath() );
			return 1;
		}
		LineCount = 1;
		return 0;
	}
}
