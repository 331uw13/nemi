#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

void freeif(void* ptr) {
    if(ptr) {
        free(ptr);
    }
}


char* get_escape_seq_args(struct escape_seq* es, char* buf, char* ptr, size_t size) {
    if(!es || !ptr || !buf || !size) {
        return NULL;
    }
    
    es->num_args = 0;

    if(*ptr == 0x1B) {
        ptr++;
    }

    if(*ptr == '[') {
        ptr++;
    }

    char argbuf[16] = { 0 };
    size_t argbuf_idx = 0;

    while(ptr < buf + size) {

        if(*ptr == 'm') {
            if(argbuf_idx > 0) {
                if(es->num_args >= ESCAPE_SEQ_MAX_ARGS) {
                    break;
                }
                es->args[es->num_args++] = atoi(argbuf); 
            }
            ptr++;
            break;
        }

        if(*ptr == ';') {
            if(es->num_args >= ESCAPE_SEQ_MAX_ARGS) {
                break;
            }
            es->args[es->num_args++] = atoi(argbuf); 

            memset(argbuf, 0, argbuf_idx);
            argbuf_idx = 0;
            
            ptr++;
            continue;
        }

        if(argbuf_idx >= sizeof(argbuf)) {
            memset(argbuf, 0, argbuf_idx);
            argbuf_idx = 0;
        }
        argbuf[argbuf_idx++] = *ptr;



        ptr++;
    }

    return ptr;
}


