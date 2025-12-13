#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

void freeif(void* ptr) {
    if(ptr) {
        free(ptr);
    }
}


void print_literal(const char* text, size_t len) {
   
    for(size_t i = 0; i <= len; i++) {
        char ch = text[i];

        if(ch == 0x1B) { // ESC
            printf("\033[34m^[\033[0m");
            continue;
        }
        else
        if(ch == 0x07) { // BEL
            printf("\033[34m^G\033[0m");
            continue;
        }
        else
        if(ch == 0x0A) {
            printf("\033[34m\\n\033[0m");
        }
        else
        if(ch < 0x20 || ch > 0x7E) {
            printf("\033[31m%x\033[0m", ch);
            continue;
        }

        printf("%c", ch);
    }
    printf("\n");
    printf("\033[90m––––––––––––––––––-----------------------------------\033[0m\n");
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

    if(*ptr == '?') {
        // Skip private mode sequences for now..
        while(ptr < buf + size) {
            if((*ptr == 'l') || (*ptr == 'h')) {
                ptr++;
                break;
            }
            ptr++;
        }

        return ptr;
    }

    if(*ptr == ']') {
        // OSC sequence begins.
        
        ptr++;
        es->num_args++;
        es->args[0] = OSC_ESCSEQ_BEGIN;
        return ptr;
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

bool chararray_contains(char* buf, size_t size, char ch) {
    char* ptr = &buf[0];
    while(ptr < buf + size) {
        if(*ptr == ch) {
            return true;
        }
        ptr++;
    }
    return false;
}

