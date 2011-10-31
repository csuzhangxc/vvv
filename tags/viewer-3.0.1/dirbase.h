// (c) by Stefan Roettger

#ifndef DIRBASE_H
#define DIRBASE_H

// specify file search path and pattern (with '*' as single wildcard)
void filesearch(const char *spec=NULL);

// find next file matching the search pattern
const char *findfile();

#endif
