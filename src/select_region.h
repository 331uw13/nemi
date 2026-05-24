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
