#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))
#define STR_MATCH(A, B) (strcmp(A, B) == 0)
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

#define HEX2RED_CHANNEL(num) ((num & 0xFF0000) >> 16)
#define HEX2GRN_CHANNEL(num) ((num & 0x00FF00) >> 8)
#define HEX2BLU_CHANNEL(num)  (num & 0x0000FF)




void freeif(void* ptr);
int  clampi(int x, int min, int max);
bool chararray_contains(char* buf, size_t size, char ch);



#endif
