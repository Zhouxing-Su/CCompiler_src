/*
* string constant is not taken into consideration temporary.
*/

%option prefix="ir"
%option never-interactive
%option nounistd

digit			[0-9]
letter		[_A-Za-z]
number		({digit}+(\.)?{digit}*)|({digit}*(\.)?{digit}+)

word		{letter}({letter}|{digit})*

userVar		"%"{word}
tempVar		"%@"{word}
constant	"#"{number}

%{
	#include <cstdlib>
	#include <string>
	#include "Log.h"
	#include "CodeGenerator.h"
	#include "ir.tab.hpp"

	using namespace std;
	
	extern "C" {
		int irwrap(void);
	}

	int line = 1;
%}

%%

"{"|"}"|"["|"]"|","|":"	{	return *irtext;	}

"MOV"	{	return MOV;	}
"ADD"	{	return ADD;	}
"SUB"	{	return SUB;	}
"MUL"	{	return MUL;	}
"DIV"	{	return DIV;	}
"MOD"	{	return MOD;	}
"INC"	{	return INC;	}
"DEC"	{	return DEC;	}

"AND"	{	return AND;	}
"OR"	{	return OR;	}
"XOR"	{	return XOR;	}
"NEG"	{	return NEG;	}
"LSHL"	{	return LSHL;	}
"LSHR"	{	return LSHR;	}
"ASHL"	{	return ASHL;	}
"ASHR"	{	return ASHR;	}

"EQ"	{	return EQ;	}
"NE"	{	return NE;	}
"GT"	{	return GT;	}
"LT"	{	return LT;	}
"GE"	{	return GE;	}
"LE"	{	return LE;	}

"LEA"	{	return LEA;	}
"DREF"	{	return DREF;	}
"CALL"	{	return CALL;	}
"INVOKE"	{	return INVOKE;	}
"RET"	{	return RET;	}
"JCT"	{	return JCT;	}
"JCF"	{	return JCF;	}
"JMP"	{	return JMP;	}


"c1"	{	return C1;	}
"f8"	{	return F8;	}
"f4"	{	return F4;	}
"i8"	{	return I8;	}
"i4"	{	return I4;	}
"i2"	{	return I2;	}
"u8"	{	return U8;	}
"u4"	{	return U4;	}
"u2"	{	return U2;	}
"s"{number}	{	return STR;	}


{userVar}	{
	irlval.pstr = new std::string(irtext);
	return USER_VAR;
}

{tempVar}	{
	irlval.pstr = new std::string(irtext);
	return TEMP_VAR;
}

{constant}	{
	irlval.pstr = new std::string(irtext);
	return CONSTANT;
}



{word}	{		// labels and function names
	irlval.pstr = new std::string(irtext);
	return ID;
}




\n 	{	++line;	}
[ \t\r]	{	;	}

.	{	fprintf( stderr, "Error occuring at line %d !\n", line );	}

%%

int irwrap(void)
{
	static int count = 0;
	string fn(CodeGenerator::cg->folder + "/IR.imp");
	if ( count == 1 ) {
		return 1;
	} else {
		if ( NULL == freopen( fn.c_str(), "r", stdin ) ) {
			Log::FileNotFoundError( fn.c_str() );
			return 1;
		}
		line = 1;
		++count;
		return 0;
	}
}
