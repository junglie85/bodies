#ifndef LOG_H
#define LOG_H

typedef enum log_category_t log_category_t;
enum log_category_t
{
    LOG_CATEGORY_APPLICATION,
    LOG_CATEGORY_GPU,
    LOG_CATEGORY_MEMORY,
    LOG_CATEGORY_WINDOW,
};

void start_log_system(void);

void log_debug(log_category_t category, const char *fmt, ...);

void log_error(log_category_t category, const char *fmt, ...);

void log_info(log_category_t category, const char *fmt, ...);

void log_trace(log_category_t category, const char *fmt, ...);

void log_warn(log_category_t category, const char *fmt, ...);

#endif // LOG_H
