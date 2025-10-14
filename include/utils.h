

#ifndef __UTILS_H
#define __UTILS_H

#define VERSION 01000 

#include <stdio.h>
#include <stdlib.h>

#define LOOP for(;;)

#define ERROR(fmt, ...) \
    do { \
        fprintf(stderr, "[ERROR] %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define TODO(fmt, ...) \
    do { \
        ERROR("[TODO] " fmt "!\n", ##__VA_ARGS__); exit(EXIT_FAILURE); \
    } while (0)

#define UNREACHABLE \
    do { \
        ERROR("Should be UNREACHABLE!\n"); exit(EXIT_FAILURE); \
    } while (0)

#endif /* !__UTILS_H */ 