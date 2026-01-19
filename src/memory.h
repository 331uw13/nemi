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
