#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "string.h"


#define STR_DEFMEMSIZE 64
#define STR_REALLOC_BYTES 64



// Allocates more memory for string if needed.
static bool string_memcheck(struct string_t* str, uint32_t size_add) {
    if(str->bytes && (str->size + size_add < str->mem_size)) {
        return true;
    }

    uint32_t new_size = str->mem_size + size_add + STR_REALLOC_BYTES;
    char* new_ptr = realloc(str->bytes, new_size);
    
    if(!new_ptr) {
        fprintf(stderr, "%s:%i '%s()': %s\n",
                __FILE__,
                __LINE__,
                __func__,
                strerror(errno));
        return false;
    }

    str->bytes = new_ptr;
    str->mem_size = new_size;

    return true;
}


struct string_t string_create(uint32_t initial_size) {
    struct string_t str;

    str.mem_size = (initial_size == 0) ? STR_DEFMEMSIZE : initial_size;
    str.bytes = calloc(1, str.mem_size);
    str.size = 0;

    return str;
}

void free_string(struct string_t* str) {
    if(str->bytes) {
        free(str->bytes);
        str->bytes = NULL;
        str->mem_size = 0;
        str->size = 0;
    }
}

void string_nullterm(struct string_t* str) {
    if(!str->bytes) {
        return;
    }
    if(str->bytes[str->size] == '\0') {
        return;
    }
    
    if(!string_memcheck(str, 1)) {
        return;
    }

    str->bytes[str->size] = '\0';
}

void string_move(struct string_t* str, char* data, int32_t size) {
    if(size < 0) {
        size = strlen(data);
    }

    if(!string_memcheck(str, size)) {
        return;
    }

    string_clear(str);
    memmove(str->bytes, data, size);
    str->size = size;
}

void string_pushbyte(struct string_t* str, char ch) {
    if(!string_memcheck(str, 1)) {
        return;
    }

    str->bytes[str->size] = ch;
    str->size += 1;
}

void string_clear(struct string_t* str) {
    if(!str->bytes) {
        return;
    }

    memset(str->bytes, 0, (str->size < str->mem_size) ? str->size : str->mem_size);
    str->size = 0;
}

void string_reserve(struct string_t* str, uint32_t size) {
    string_memcheck(str, size);
}


char string_lastbyte(struct string_t* str) {
    if(!str) {
        return 0;
    }
    if(!str->bytes) {
        return 0;
    }
    if(str->size >= str->mem_size) {
        return 0;
    }

    return str->bytes[(str->size > 0) ? str->size-1 : 0];
}

bool string_append(struct string_t* str, char* data, int32_t size) {
    if(size < 0) {
        size = strlen(data);
    }

    if(!string_memcheck(str, size)) {
        return false;
    }

    memmove(str->bytes + str->size, data, size);
    str->size += size;
    
    return true;
}

ssize_t string_charptr_find(char* data, size_t data_size, char* part, size_t part_size) {
    ssize_t found_index = -1;

    if(part_size == 0) {
        goto skip;
    }
    if(data_size < part_size) {
        goto skip;
    }

    char* ch = &data[0];
    while(ch < data + data_size) {
        if(*ch == part[0]) {
            if(ch + part_size > data + data_size) {
                break; // Prevent out of bounds read.
            }

            bool found = true;
           
            // First character of 'part' was found, check if rest match.
            for(size_t pi = 0; pi < part_size; pi++) {
                if(*ch != part[pi]) {
                    found = false;
                    break;
                }
                ch++;
            }
            if(found) {
                found_index = (ch - data) - part_size;
                break;
            }
        }
        ch++;
    }

skip:
    return found_index;
}


