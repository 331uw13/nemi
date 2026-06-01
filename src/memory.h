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
#ifndef MEMORY_UTIL_H
#define MEMORY_UTIL_H



struct reallocz_args {
    size_t  element_size;       // Element size in bytes.
    size_t* num_elements_alloc; // How many elements is currently allocated.
    size_t  num_elements_add;   // How many elements are going to be added.
    size_t  num_elements;       // How many elements are currently used.
};

// Calls realloc and sets new memory region to zero.
// 'num_elements_alloc' is incremented before returning new pointer.
// The pointer is returned untouched if no need to reallocate memory.
void* nemi_reallocz_ifneed(void* ptr, struct reallocz_args args);

// Free memory if 'ptr' is not NULL.
void freeif(void* ptr);

#endif
