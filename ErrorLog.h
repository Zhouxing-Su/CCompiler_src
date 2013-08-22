#ifndef ERROR_LOG_H
#define ERROR_LOG_H 0

#include <cstdio>
#include "FileWraper.h"

class ErrorLog
{
public:
    static void FileNotFoundError( char * path ) {
        fprintf( stderr, "Error : Could not find file \"%s\"!\n", path );
    }

    static void InvalidCharacherWarning( int LineCount, char *text ) {
        fprintf( stderr, "line %d : invalid character(s)£º%s\n", LineCount, text );
    }

};

#endif

