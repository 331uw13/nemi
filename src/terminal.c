#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pty.h>
#include <sys/select.h>


#include "terminal.h"
#include "nemi.h"



struct terminal* spawn_terminal(struct nemi* st) {
    if(st->num_terminals+1 >= NEMI_TERMINALS_MAX) {
        return NULL;
    }

    struct terminal* term = &st->terminals[st->num_terminals++];
    term->pid = forkpty(&term->master_fd, NULL, NULL, NULL);

    if(term->pid == 0) {
        execlp(getenv("SHELL"), getenv("SHELL"), NULL);
    }

    return term;
}

void close_terminal(struct terminal* term) {
    if(!term) {
        return;
    }
    if(term->master_fd < 0) {
        return;
    }

    close(term->master_fd);
    term->master_fd = -1;
}

void read_terminal(struct terminal* term, size_t* read_bytes, char* out, size_t mem_size) {
    if(read_bytes) {
        *read_bytes = 0;
    }
    if(!term) {
        return;
    }
    if(!out || !mem_size) { 
        return;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(term->master_fd, &fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10 * 1000;

    int retv = select(term->master_fd+1, &fds, NULL, NULL, &timeout);
    if(retv > 0) {
        ssize_t rd = read(term->master_fd, out, mem_size);
        if(read_bytes && rd > 0) {
            *read_bytes = (size_t)rd;
        }
    }
}

void execute_cmd(struct terminal* term, const char* cmd_str) {
    if(!term) {
        return;
    }

    write(term->master_fd, cmd_str, strlen(cmd_str));
}





