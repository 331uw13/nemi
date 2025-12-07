#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))


void freeif(void* ptr);





#define ESCAPE_SEQ_MAX_ARGS 16

struct escape_seq {
    int      args [ESCAPE_SEQ_MAX_ARGS];
    uint16_t num_args;
};

// Returns new pointer to 'buf' where to continue reading.
// 'NULL' is returned if at end of buffer.
// 'ptr' is supposed to be where to start reading.
char* get_escape_seq_args(struct escape_seq* es, char* buf, char* ptr, size_t size);




#endif
