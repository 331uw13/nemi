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
#ifndef NEMI_SELECT_REGION_H
#define NEMI_SELECT_REGION_H


#include <sys/types.h>
#include <stddef.h>


typedef enum NSelectRegionMode_e {
    SREG_MODE_NORMAL,
    SREG_MODE_LINE,
    SREG_MODE_BLOCK
}
NSelectRegionMode;

typedef struct NSelectRegion_t {
    ssize_t col_beg;
    ssize_t row_beg;
    ssize_t col_end;
    ssize_t row_end;
    
    bool active;
    bool is_orientated;
    NSelectRegionMode mode;
}
NSelectRegion;


// TODO: Swap col and row.

void nmt_select_begin              (NSelectRegion* reg, int col_begin, int row_begin);
void nmt_select_move               (NSelectRegion* reg, int col, int row);
NSelectRegion nmt_select_orientate (const NSelectRegion* reg);


void nmt_select_process
(
    NSelectRegion reg,
    size_t(*callback_get_row_length)
    (
        void* user_pointer,
        ssize_t row
    ),
    void(*callback)
    (
        void* user_pointer,
        ssize_t row,
        ssize_t row_length,
        ssize_t col_begin
    ),
    void* user_pointer
);


#endif
