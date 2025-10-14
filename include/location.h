

#ifndef __LOCATION_H
#define __LOCATION_H

#include <stddef.h>

typedef struct location
{
    /* source file name */
    char *filename;

    /* line number (1-based) */
    size_t line;

    /* column number (1-based) */
    size_t column;
} location_t;

#define LOCATION(_location) \
    (_location).filename, (_location).line, (_location).column


#endif /* !__LOCATION_H */