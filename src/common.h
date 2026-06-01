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
#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))
#define STR_MATCH(a, b) (strcmp(a, b) == 0)
#define MIN_VALUE(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_VALUE(a, b) (((a) > (b)) ? (a) : (b))

#define HEX2RED_CHANNEL(num) (((num) & 0xFF0000) >> 16)
#define HEX2GRN_CHANNEL(num) (((num) & 0x00FF00) >> 8)
#define HEX2BLU_CHANNEL(num)  ((num) & 0x0000FF)



int  clampi(int x, int min, int max);
bool chararray_contains(char* buf, size_t size, char ch);



#endif
