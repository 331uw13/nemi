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
#ifndef NEMI_LOG_H
#define NEMI_LOG_H


// Log levels
#define LOG_INFO  (1 << 0)
#define LOG_WARN  (1 << 1)
#define LOG_ERROR (1 << 2)

// Log settings
#define LOG_ENABLED         (1 << 3)
#define LOG_INCLUDE_CALLEE  (1 << 4)
#define LOG_USE_COLOR       (1 << 5)


struct log_settings {
    int flags;
    char output [256];
};

void log_init(struct log_settings settn);
void log_close();

void logprintf_ex
(
    int log_level,
    const char* callee_func,
    const char* callee_file,
    const int   callee_file_line,
    const char* message,
    ...
);

#define logprintf(log_level, message, ...) \
    logprintf_ex(log_level, __func__, __FILE__, __LINE__, message, ##__VA_ARGS__)



#endif
