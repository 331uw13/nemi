#ifndef STRING_UTIL_H
#define STRING_UTIL_H


#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>


struct string_t {
    char* bytes;
    uint32_t size;
    uint32_t mem_size;
};


// If initial_size is equal to 0. Default is used.
struct string_t string_create(uint32_t initial_size);
void free_string(struct string_t* str);

// Makes sure the str->bytes is null terminated.
// Note: str->size is not increased even if null byte was added.
void string_nullterm(struct string_t* str);

// Clear and move 'data' to beginning of 'str'
// If 'size' is negative. 'data' must be null terminated.
void string_move(struct string_t* str, char* data, int32_t size);

// Add byte to end of string.
void string_pushbyte(struct string_t* str, char ch);

// Sets all 'str->size' bytes to 0
void string_clear(struct string_t* str);

// Makes sure str can hold 'size' number of bytes.
void string_reserve(struct string_t* str, uint32_t size);

// Get last byte of str->data
char string_lastbyte(struct string_t* str);

// Append 'data' at end of string.
// If 'size' is negative. 'data' must be null terminated.
bool string_append(struct string_t* str, char* data, int32_t size);



// ==== Miscellaneous utils ====

// Find 'part' starting index in 'data'
// if not found -1 is returned.
ssize_t string_charptr_find(char* data, size_t data_size, char* part, size_t part_size);


#endif
