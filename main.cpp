#include <cstdio>
#include "FileWraper.h"
#include "ErrorLog.h"
#include "Symbol.h"

int init( int argc, char **argv );
void generateIntermediateCode();
int yyparse(void);


int main( int argc, char **argv ) {

    if ( init( argc, argv ) ) {
        return -1;
    }

    generateIntermediateCode();

    //optimizeIntermediateCode();

    //generateObjectCode();

    return 0;
}

int init( int argc, char **argv ) {
    // link the file path to FileWraper
    FileWraper::fw = new FileWraper( argc, argv );

    if ( NULL == freopen( FileWraper::getPath(), "r", stdin ) ) {
        ErrorLog::FileNotFoundError( FileWraper::getPath() );
        return -1;
    }

    // initializtion about the major data structure
    Label::initNameBuf();

    VarType::initAtomTypes();

    return 0;
}

void generateIntermediateCode() {
    yyparse();
}
