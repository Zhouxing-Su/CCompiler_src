#include <cstdio>
#include "FileWraper.h"
#include "Log.h"
#include "SymbolTable.h"

int init( int argc, char **argv );
void generateIntermediateCode();
int yyparse(void);


int main( int argc, char **argv ) {

    if ( init( argc, argv ) ) {
        return -1;
    }

    //preprocessing();

    generateIntermediateCode();

    //optimizeIntermediateCode();

    //generateObjectCode();

    return 0;
}

int init( int argc, char **argv ) {
    // link the file path to FileWraper
    FileWraper::fw = new FileWraper( argc, argv );
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
