#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))
#define STR_MATCH(A, B) (strcmp(A, B) == 0)



void freeif(void* ptr);
int  clampi(int x, int min, int max);
bool chararray_contains(char* buf, size_t size, char ch);



#endif
