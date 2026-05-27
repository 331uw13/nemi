#include <assert.h>

#include "select_region.h"





void nmt_select_begin(NSelectRegion* reg, int col_begin, int row_begin) {
    reg->col_beg = col_begin;
    reg->row_beg = row_begin;
    reg->col_end = reg->col_beg;
    reg->row_end = reg->row_beg;
    reg->active = true;
    reg->is_orientated = false;
}

void nmt_select_move(NSelectRegion* reg, int col, int row) {
    if(!reg->active) {
        return;
    }

    reg->col_end = col;
    reg->row_end = row;
    reg->is_orientated = false;
}

NSelectRegion nmt_select_orientate(const NSelectRegion* reg) {
    NSelectRegion o_reg;
    o_reg.active = reg->active;
    o_reg.mode = reg->mode;

    if(reg->row_beg > reg->row_end) {
        o_reg.row_beg = reg->row_end;
        o_reg.row_end = reg->row_beg;

        o_reg.col_end = reg->col_beg;
        o_reg.col_beg = reg->col_end;
    }
    else {
        o_reg.row_beg = reg->row_beg;
        o_reg.row_end = reg->row_end;
        o_reg.col_beg = reg->col_beg;
        o_reg.col_end = reg->col_end;
    }

    o_reg.is_orientated = true;
    return o_reg;
}

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
){
    assert(callback_get_row_length != NULL);
    assert(callback != NULL);


    if(!reg.is_orientated) {
        reg = nmt_select_orientate(&reg);
    }

    for(ssize_t row = reg.row_beg; row <= reg.row_end; row++) {
        ssize_t row_length = callback_get_row_length(user_pointer, row);
        const ssize_t row_length_original = row_length;
        ssize_t col_begin = 0;

        switch(reg.mode) {

            case SREG_MODE_NORMAL:
                {
                    if(row_length == 0) {
                        callback
                        (
                            user_pointer,
                            row,
                            0,
                            col_begin
                        );
                        break;
                    }

                    if(row == reg.row_beg && row == reg.row_end) {
                        // The select is at the same row.
                        
                        if(reg.col_beg < reg.col_end) {
                            col_begin = reg.col_beg;
                            row_length = reg.col_end - reg.col_beg;
                        }
                        else {
                            col_begin = reg.col_end;
                            row_length = reg.col_beg - reg.col_end;
                        }
                    }
                    else
                    if(row == reg.row_beg) {
                        col_begin = reg.col_beg;
                        row_length -= col_begin;
                    }
                    else 
                    if(row == reg.row_end) {
                        row_length -= (row_length - reg.col_end);
                    }

                    if(row_length <= 0) {
                        break; // TODO: Pass zero length rows to callback (?)
                    }

                    if(row_length > row_length_original) {
                        row_length = row_length_original;
                    }

                    callback
                    (
                        user_pointer,
                        row,
                        row_length,
                        col_begin
                    );
                }
                break;

            case SREG_MODE_LINE:
                {
                    callback
                    (
                        user_pointer,
                        row,
                        row_length,
                        col_begin
                    );
                }
                break;
            
            case SREG_MODE_BLOCK:
                {
                    if(reg.col_beg < reg.col_end) {
                        col_begin = reg.col_beg;
                        row_length = reg.col_end - reg.col_beg;
                    }
                    else {
                        col_begin = reg.col_end;
                        row_length = reg.col_beg - reg.col_end;
                    }

                    callback
                    (
                        user_pointer,
                        row,
                        row_length,
                        col_begin
                    );
                }
                break;
        }
    }

}

