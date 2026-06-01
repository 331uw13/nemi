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



void freeif(void* ptr) {
    if(ptr) {
        free(ptr);
    }
}

