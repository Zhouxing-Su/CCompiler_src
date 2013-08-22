/**
   this class manage all input source file,
   and make it easy for the yylex() to handle them one by one.
*/

#ifndef FILE_WRAPER_H
#define FILE_WRAPER_H 0

#include <cstdlib>

class FileWraper
{
public:
	friend int init( int argc, char **argv );

	static char *getPath(void);
	static char *nextPath(void);

private:
	// the constructor should not be called outside the main.cpp::init() function
	FileWraper( int argc, char **argv );
	~FileWraper();

	static FileWraper *fw;

	int curFileIndex;
	int count;
	char **path;
};

#endif
