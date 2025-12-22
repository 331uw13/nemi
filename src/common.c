#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

void freeif(void* ptr) {
    if(ptr) {
        free(ptr);
    }
}

int clampi(int x, int min, int max) {
    if(x < min) {
        return min;
    }
    if(x > max) {
        return max;
    }
    return x;
}

bool chararray_contains(char* buf, size_t size, char ch) {
    char* ptr = &buf[0];
    while(ptr < buf + size) {
        if(*ptr == ch) {
            return true;
        }
        ptr++;
    }
    return false;
}



