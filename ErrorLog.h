#include <cstdio>
#include "FileWraper.h"

#ifndef ERROR_LOG_H
#define ERROR_LOG_H 0

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

