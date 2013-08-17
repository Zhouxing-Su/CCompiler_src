#include "FileWraper.h"

FileWraper::FileWraper( int argc, char **argv ) : curFileIndex(0), count(argc-1) {
	path = (char **)malloc( sizeof(char *) * count);
	for( int i = 0 ; i < count ; ++i ) {
		path[i] = argv[i+1];
	}
}

FileWraper::~FileWraper() {
	free(path);
}
		
char * FileWraper::getPath(void) {
	return fw->path[fw->curFileIndex];
}
	
char * FileWraper::nextPath() {
	if( (++fw->curFileIndex) < fw->count ) {
		return fw->path[++fw->curFileIndex];
	} else {
		return NULL;
	}
}

FileWraper * FileWraper::fw = NULL;