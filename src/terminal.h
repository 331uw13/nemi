#ifndef TERMINAL_H
#define TERMINAL_H

#include <pty.h>
#include <stddef.h>



struct terminal {
    int    master_fd;
    pid_t  pid; // Child process PID.


};



struct nemi;


struct terminal* spawn_terminal(struct nemi* st);
void             close_terminal(struct terminal* term);
void             read_terminal(struct terminal* term, size_t* read_bytes, char* out, size_t mem_size);
void             execute_cmd(struct terminal* term, const char* cmd_str);


#endif
