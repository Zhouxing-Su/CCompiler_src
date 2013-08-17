#include <cstdio>
#include "FileWraper.h"
#include "ErrorLog.h"

int init( int argc, char **argv );
void generateIntermediateCode();
int yyparse(void);


int main( int argc, char **argv ) {

    if ( init( argc, argv ) ) {
        return -1;
    }

    generateIntermediateCode();

    //optimizeIntermediateCode();

    return 0;
}

int init( int argc, char **argv ) {

    FileWraper::fw = new FileWraper( argc, argv );

    if ( NULL == freopen( FileWraper::getPath(), "r", stdin ) ) {
        ErrorLog::FileNotFoundError( FileWraper::getPath() );
        return -1;
    }

    //freopen( "out.txt", "w", stdout );

    return 0;
}

void generateIntermediateCode() {
    yyparse();
}
