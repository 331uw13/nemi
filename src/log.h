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
