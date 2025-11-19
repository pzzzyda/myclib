#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include "myclib/log.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <process.h>
typedef CRITICAL_SECTION mc_mutex_t;
#define MC_MUTEX_INIT(mutex) InitializeCriticalSection(mutex)
#define MC_MUTEX_LOCK(mutex) EnterCriticalSection(mutex)
#define MC_MUTEX_UNLOCK(mutex) LeaveCriticalSection(mutex)
#define MC_MUTEX_DESTROY(mutex) DeleteCriticalSection(mutex)
#define MC_GETPID() _getpid()
#define MC_GETTID() GetCurrentThreadId()
#else
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
typedef pthread_mutex_t mc_mutex_t;
#define MC_MUTEX_INIT(mutex) pthread_mutex_init(mutex, NULL)
#define MC_MUTEX_LOCK(mutex) pthread_mutex_lock(mutex)
#define MC_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(mutex)
#define MC_MUTEX_DESTROY(mutex) pthread_mutex_destroy(mutex)
#define MC_GETPID() getpid()
#define MC_GETTID() pthread_self()
#endif

#define MC_ANSI_COLOR_RED "\x1b[31m"
#define MC_ANSI_COLOR_GREEN "\x1b[32m"
#define MC_ANSI_COLOR_YELLOW "\x1b[33m"
#define MC_ANSI_COLOR_BLUE "\x1b[34m"
#define MC_ANSI_COLOR_MAGENTA "\x1b[35m"
#define MC_ANSI_COLOR_CYAN "\x1b[36m"
#define MC_ANSI_COLOR_RESET "\x1b[0m"

struct mc_logger {
    FILE *log_file;
    mc_mutex_t mutex;
    struct mc_logger_config cfg;
};

#define MC_LOGGER_LOCK(logger)                                                 \
    do {                                                                       \
        if (((logger)->cfg.options & MC_LOG_OPTION_THREAD_SAFE))               \
            MC_MUTEX_LOCK(&(logger)->mutex);                                   \
    } while (0)

#define MC_LOGGER_UNLOCK(logger)                                               \
    do {                                                                       \
        if (((logger)->cfg.options & MC_LOG_OPTION_THREAD_SAFE))               \
            MC_MUTEX_UNLOCK(&(logger)->mutex);                                 \
    } while (0)

struct mc_logger *mc_logger_new(struct mc_logger_config const *cfg)
{
    if (!cfg || !MC_LOG_LEVEL_IS_VALID(cfg->level))
        return NULL;

    struct mc_logger *logger = malloc(sizeof(*logger));
    if (!logger)
        return NULL;

    logger->log_file = NULL;
    MC_MUTEX_INIT(&logger->mutex);
    logger->cfg = *cfg;

    if (!cfg->log_file_path) {
        logger->log_file = stderr;
        return logger;
    }

    char const *const mode = (cfg->options & MC_LOG_OPTION_APPEND) ? "a" : "w";

#if defined(_WIN32) || defined(_WIN64)
    errno_t err = fopen_s(&logger->log_file, cfg->log_file_path, mode);
    if (err != 0) {
        mc_logger_free(logger);
        return NULL;
    }
#else
    logger->log_file = fopen(cfg->log_file_path, mode);
    if (!logger->log_file) {
        mc_logger_free(logger);
        return NULL;
    }
#endif

    return logger;
}

void mc_logger_free(struct mc_logger *logger)
{
    if (!logger)
        return;

    MC_LOGGER_LOCK(logger);
    if (logger->log_file && logger->log_file != stderr &&
        logger->log_file != stdout) {
        fclose(logger->log_file);
        logger->log_file = NULL;
    }
    MC_LOGGER_UNLOCK(logger);
    MC_MUTEX_DESTROY(&logger->mutex);
    free(logger);
}

void mc_logger_set_level(struct mc_logger *logger, enum mc_log_level level)
{
    if (!logger || !MC_LOG_LEVEL_IS_VALID(level))
        return;

    MC_LOGGER_LOCK(logger);
    logger->cfg.level = level;
    MC_LOGGER_UNLOCK(logger);
}

enum mc_log_level mc_logger_get_level(struct mc_logger *logger)
{
    if (!logger)
        return MC_LOG_LEVEL_INFO;

    enum mc_log_level level;
    MC_LOGGER_LOCK(logger);
    level = logger->cfg.level;
    MC_LOGGER_UNLOCK(logger);
    return level;
}

void mc_logger_set_format(struct mc_logger *logger, int format_flags)
{
    if (!logger)
        return;

    MC_LOGGER_LOCK(logger);
    logger->cfg.format_flags = format_flags;
    MC_LOGGER_UNLOCK(logger);
}

int mc_logger_get_format(struct mc_logger *logger)
{
    if (!logger)
        return MC_LOG_FORMAT_DEFAULT;

    int format_flags;
    MC_LOGGER_LOCK(logger);
    format_flags = logger->cfg.format_flags;
    MC_LOGGER_UNLOCK(logger);
    return format_flags;
}

static size_t mc_logger_timestamp(char *buf, size_t buf_len)
{
    time_t now;
    struct tm tm_info;
    struct tm *tm_ptr;

    time(&now);

#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_info, &now);
    tm_ptr = &tm_info;
#else
    tm_ptr = localtime_r(&now, &tm_info);
#endif

    if (tm_ptr == NULL)
        return 0;

    return strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S", tm_ptr);
}

static char const *mc_logger_level_color(enum mc_log_level level, int use_color)
{
    if (!use_color)
        return "";

    switch (level) {
    case MC_LOG_LEVEL_TRACE:
        return MC_ANSI_COLOR_MAGENTA;
    case MC_LOG_LEVEL_DEBUG:
        return MC_ANSI_COLOR_BLUE;
    case MC_LOG_LEVEL_INFO:
        return MC_ANSI_COLOR_GREEN;
    case MC_LOG_LEVEL_WARN:
        return MC_ANSI_COLOR_YELLOW;
    case MC_LOG_LEVEL_ERROR:
        return MC_ANSI_COLOR_RED;
    case MC_LOG_LEVEL_FATAL:
        return MC_ANSI_COLOR_RED;
    default:
        return "";
    }
}

static char const *mc_logger_level_str(enum mc_log_level level)
{
    switch (level) {
    case MC_LOG_LEVEL_TRACE:
        return "[TRACE]";
    case MC_LOG_LEVEL_DEBUG:
        return "[DEBUG]";
    case MC_LOG_LEVEL_INFO:
        return "[INFO ]";
    case MC_LOG_LEVEL_WARN:
        return "[WARN ]";
    case MC_LOG_LEVEL_ERROR:
        return "[ERROR]";
    case MC_LOG_LEVEL_FATAL:
        return "[FATAL]";
    default:
        return "[UNKNOWN]";
    }
}

void mc_logger_write(struct mc_logger *logger, enum mc_log_level level,
                     char const *file, int line, char const *fmt, ...)
{
    va_list args;

    if (!logger || !fmt || !MC_LOG_LEVEL_IS_VALID(level))
        return;

    MC_LOGGER_LOCK(logger);
    if (level < logger->cfg.level) {
        MC_LOGGER_UNLOCK(logger);
        return;
    }

    va_start(args, fmt);

    bool const use_color =
        (logger->cfg.options & MC_LOG_OPTION_COLOR_OUTPUT) &&
        (logger->log_file == stdout || logger->log_file == stderr);

    if (logger->cfg.format_flags & MC_LOG_FORMAT_TIME) {
        char time_str[64] = {0};
        size_t time_str_len = mc_logger_timestamp(time_str, sizeof(time_str));
        fwrite(time_str, time_str_len, sizeof(char), logger->log_file);
        fputc(' ', logger->log_file);
    }

    if (logger->cfg.format_flags & MC_LOG_FORMAT_PID)
        fprintf(logger->log_file, "[PID:%lu] ", (unsigned long)MC_GETPID());

    if (logger->cfg.format_flags & MC_LOG_FORMAT_TID) {
#if defined(_WIN32) || defined(_WIN64)
        fprintf(logger->log_file, "[TID:%lu] ", (unsigned long)MC_GETTID());
#else
        fprintf(logger->log_file, "[TID:%lu] ", (unsigned long)MC_GETTID());
#endif
    }

    if (logger->cfg.format_flags & MC_LOG_FORMAT_LEVEL) {
        char const *color = mc_logger_level_color(level, use_color);
        char const *reset = use_color ? MC_ANSI_COLOR_RESET : "";
        char const *level_str = mc_logger_level_str(level);

        fprintf(logger->log_file, "%s%s%s ", color, level_str, reset);
    }

    if (logger->cfg.format_flags & MC_LOG_FORMAT_FILE) {
        char const *base_file = strrchr(file, '/');
        base_file = base_file ? base_file + 1 : file;
#if defined(_WIN32) || defined(_WIN64)
        char const *win_base_file = strrchr(file, '\\');
        if (win_base_file)
            base_file = win_base_file + 1;

#endif
        fprintf(logger->log_file, "%s:%d: ", base_file, line);
    }

    vfprintf(logger->log_file, fmt, args);
    fputc('\n', logger->log_file);

    if (logger->cfg.options & MC_LOG_OPTION_FLUSH_IMMEDIATE)
        fflush(logger->log_file);

    va_end(args);
    MC_LOGGER_UNLOCK(logger);
}

void mc_logger_flush(struct mc_logger *logger)
{
    if (!logger)
        return;

    MC_LOGGER_LOCK(logger);
    fflush(logger->log_file);
    MC_LOGGER_UNLOCK(logger);
}
