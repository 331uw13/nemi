#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))
#define STR_MATCH(A, B) (strcmp(A, B) == 0)



void freeif(void* ptr);
int  clampi(int x, int min, int max);
void print_literal(const char* text, size_t len);
bool chararray_contains(char* buf, size_t size, char ch);


#define ESCAPE_SEQ_MAX_ARGS 16
#define OSC_ESCSEQ_BEGIN 0xFFFF

struct escape_seq {
    int      args [ESCAPE_SEQ_MAX_ARGS];
    uint16_t num_args;
};

// TODO: Rename this function.
// This function splits ANSI escape sequence
// arguments into integers which can later be processed.
// For example:
// "\x1b[2;32m" becomes: [ 2, 32 ]
// When 'm'--^ is the current character being read,
// it returns new pointer to 'buf' where to continue normal reading.
// 'ptr' is supposed to be where to start.
char* get_escape_seq_args(struct escape_seq* es, char* buf, char* ptr, size_t size);



#endif
