#ifndef ERROR_LOG_H
#define ERROR_LOG_H 0

#include <cstdio>
#include <string>
#include "FileWraper.h"

extern int LineCount;

/**
    Error : 
    Warning : 
    Log : 
*/
class ErrorLog
{
public:
    static void FileNotFoundError( const char *path ) {
        fprintf( stderr, "Error : Could not find file \"%s\"!\n", path );
    }

    static void CompoundLevelConflictError( const std::string &name, const char *type ) {
        fprintf( stderr, "Error: %s is not a %s (%s[line %d])\n", name.c_str(), type, FileWraper::getPath(), LineCount );
    }

    static void InvalidCharacherError( const char *text ) {
        fprintf( stderr, "Error: invalid characters£º%s (%s[line %d])\n", text, FileWraper::getPath(), LineCount );
    }

    static void syntaxAnalasisLog( const char *msg, int linenoShift ) {
        printf( "%s[line%5d]: %s\n", FileWraper::getPath(), (LineCount + linenoShift), msg );
    }

};

#endif

