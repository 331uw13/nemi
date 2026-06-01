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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "log.h"
#include "common.h"


static struct global_log {
    int flags;
    int fd;
    
    char    msgbuf[512];
    ssize_t msgbuf_len;

    char    calleebuf[256];
    ssize_t calleebuf_len;

    char    levelbuf[64];
    ssize_t levelbuf_len;
}
log = (struct global_log) {
    .flags = 0,
    .fd = -1
};


void log_init(struct log_settings settn) {
    log.flags = settn.flags;

    if(!STR_MATCH(settn.output, "<print>")) {
        log.fd = open(settn.output, 
                O_WRONLY | O_CREAT | O_APPEND,
                S_IRUSR  | S_IWUSR);
    
        if(log.fd <= 0) {
            fprintf(stderr, "%s: open: %s\n", 
                    __func__, strerror(errno));
        }
    }
    else {
        log.fd = STDERR_FILENO;
    }

    memset(log.levelbuf, 0, sizeof(log.levelbuf));
    memset(log.msgbuf, 0, sizeof(log.msgbuf));
    memset(log.calleebuf, 0, sizeof(log.calleebuf));
    log.msgbuf_len = 0;
    log.calleebuf_len = 0;
}

void log_close() {
        
    if(log.fd >= 0 && 
            (log.fd != STDOUT_FILENO && log.fd != STDERR_FILENO)) {
        close(log.fd);
    }
}


static
const char* log_level_to_str(int log_level) {
    if(log.flags & LOG_USE_COLOR) {
        switch(log_level) {
            case LOG_INFO: return "\033[1;34mInfo\033[0m";
            case LOG_WARN: return "\033[1;33mWarning\033[0m";
            case LOG_ERROR: return "\033[1;31mError\033[0m";
        }
    }
    else {
        switch(log_level) {
            case LOG_INFO: return "Info";
            case LOG_WARN: return "Warning";
            case LOG_ERROR: return "Error";
        }
    }

    return "<UnknownLogLevel>";
}

void logprintf_ex
(
    int log_level,
    const char* callee_func,
    const char* callee_file,
    const int   callee_file_line,
    const char* message,
    ...
){
    if(!(log.flags & LOG_ENABLED)) {
        return;    
    }
    if(!(log.flags & log_level)) {
        return;
    }

    va_list args;
    va_start(args, message);


    // Clear previous buffers.

    memset(log.levelbuf, 0, log.levelbuf_len);
    log.levelbuf_len = 0;

    memset(log.msgbuf, 0, log.msgbuf_len);
    log.msgbuf_len = 0;
    
    if(log.flags & LOG_INCLUDE_CALLEE) {
        memset(log.calleebuf, 0, log.calleebuf_len);
        log.calleebuf_len = 0;
    }


    log.levelbuf_len = snprintf(
            log.levelbuf, sizeof(log.levelbuf),
            "%*s: ", 
            (log.flags & LOG_USE_COLOR) ? 20 : 10,
            log_level_to_str(log_level));
    if(log.levelbuf_len < 0) {
        log.levelbuf_len = 0;
        return;
    }


    log.msgbuf_len = vsnprintf(log.msgbuf, sizeof(log.msgbuf), message, args);
    if(log.msgbuf_len < 0) {
        log.msgbuf_len = 0;
        return;
    }


    if(log.flags & LOG_INCLUDE_CALLEE) {
        log.calleebuf_len = snprintf(
                log.calleebuf, sizeof(log.calleebuf),
                "(from %s(),\"%s\":%d) - ", 
                    callee_func,
                    callee_file,
                    callee_file_line);
    }

    write(log.fd, log.levelbuf, log.levelbuf_len);
    if(log.flags & LOG_INCLUDE_CALLEE) {
        write(log.fd, log.calleebuf, log.calleebuf_len);
    }
    write(log.fd, log.msgbuf, log.msgbuf_len);
    write(log.fd, "\n", 1);

    va_end(args);
}


