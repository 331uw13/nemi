#include <stdlib.h>
#include <string.h>

#include "memory.h"


#define NUM_ALLOC_MORE 40 

void* nemi_reallocz_ifneed(void* ptr, struct reallocz_args args) {
    if(ptr && (args.num_elements + args.num_elements_add < *args.num_elements_alloc)) {
        //printf("\033[90m%s: No need to realloc\033[0m\n",__func__);
        return ptr;
    }

    args.num_elements_add += NUM_ALLOC_MORE;
    ptr = realloc(ptr, args.element_size * (args.num_elements + args.num_elements_add));

    memset(ptr + args.element_size * args.num_elements, 0,
            args.element_size * args.num_elements_add);

    *args.num_elements_alloc += args.num_elements_add;
        
    //printf("\033[93m%s: realloc + \033[0m\n",__func__);
    return ptr;
}


