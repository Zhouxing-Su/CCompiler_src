#include <cstdio>
#include "FileWraper.h"
#include "Log.h"
#include "SymbolTable.h"
#include "CodeGenerator.h"

int init( int argc, char **argv );
void generateIntermediateCode();
int yyparse(void);

// pass 2 argvs to main:
// the first one is the output directory
// the later ones are the input file path
int main( int argc, char **argv ) {

    if ( init( argc, argv ) ) {
        return -1;
    }

    //preprocessing();

    generateIntermediateCode();

    //optimizeIntermediateCode();

    CodeGenerator::cg->generateASM();

    return 0;
}

int init( int argc, char **argv ) {
    // init the cg and record the output directory
    CodeGenerator::cg = new CodeGenerator( std::string(argv[1]) );

    // link the file path to FileWraper
    FileWraper::fw = new FileWraper( argc-2, argv+2 );
    if ( NULL == freopen( FileWraper::getPath(), "r", stdin ) ) {
        Log::FileNotFoundError( FileWraper::getPath() );
        return -1;
    }

    // initializtion about the major data structure
    VarType::initAtomTypes();
    SymbolTable::initSymbolTableStack();

    return 0;
}

void generateIntermediateCode() {
    yyparse();
}
