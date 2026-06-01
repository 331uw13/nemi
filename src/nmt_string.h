/* License: zlib

   Copyright (C) 2026 (331uw13/eeiuwie)

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
*/
#ifndef STRING_UTIL_H
#define STRING_UTIL_H


#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>


struct string_t {
    char* bytes;
    size_t size;
    size_t mem_size;
};


// If initial_size is equal to 0. Default is used.
struct string_t string_create(size_t initial_size);
void free_string(struct string_t* str);

// Makes sure the str->bytes is null terminated.
// Note: str->size is not increased even if null byte was added.
void string_nullterm(struct string_t* str);

// Clear and move 'data' to beginning of 'str'
// If 'size' is negative. 'data' must be null terminated.
void string_move(struct string_t* str, char* data, size_t size);

// Add byte to end of string.
void string_pushbyte(struct string_t* str, char ch);

// Add byte to index but push remainder to left.
void string_addbyte(struct string_t* str, char ch, size_t index);

// Delete byte at index.
void string_delbyte(struct string_t* str, size_t index);

// Delete last byte.
void string_poplast(struct string_t* str);

// Sets all 'str->size' bytes to 0
void string_clear(struct string_t* str);

// Makes sure str can hold 'size' number of bytes.
void string_reserve(struct string_t* str, size_t size);

// Get last byte of str->data
char string_lastbyte(struct string_t* str);

// Append 'data' at end of string.
// If 'size' is negative. 'data' must be null terminated.
bool string_append(struct string_t* str, char* data, size_t size);



// ==== Miscellaneous utils ====

// Find 'part' starting index in 'data'
// if not found -1 is returned.
ssize_t string_charptr_find(char* data, size_t data_size, char* part, size_t part_size);


#endif
