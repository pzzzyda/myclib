#ifndef MYCLIB_LOG_H
#define MYCLIB_LOG_H

struct mc_logger;

enum mc_log_level {
    MC_LOG_LEVEL_TRACE,
    MC_LOG_LEVEL_DEBUG,
    MC_LOG_LEVEL_INFO,
    MC_LOG_LEVEL_WARN,
    MC_LOG_LEVEL_ERROR,
    MC_LOG_LEVEL_FATAL,
};

#define MC_LOG_LEVEL_IS_VALID(level)                                           \
    ((level) >= MC_LOG_LEVEL_TRACE && (level) <= MC_LOG_LEVEL_FATAL)

#define MC_LOG_OPTION_APPEND 0x01
#define MC_LOG_OPTION_THREAD_SAFE 0x02
#define MC_LOG_OPTION_COLOR_OUTPUT 0x04
#define MC_LOG_OPTION_FLUSH_IMMEDIATE 0x08

#define MC_LOG_FORMAT_TIME 0x01
#define MC_LOG_FORMAT_LEVEL 0x02
#define MC_LOG_FORMAT_FILE 0x04
#define MC_LOG_FORMAT_PID 0x08
#define MC_LOG_FORMAT_TID 0x10
#define MC_LOG_FORMAT_DEFAULT (MC_LOG_FORMAT_TIME | MC_LOG_FORMAT_LEVEL)

struct mc_logger_config {
    char const *log_file_path;
    int options;
    int format_flags;
    enum mc_log_level level;
};

#define MC_LOGGER_CONFIG_DEFAULT()                                             \
    {.log_file_path = NULL,                                                    \
     .options = 0,                                                             \
     .format_flags = MC_LOG_FORMAT_DEFAULT,                                    \
     .level = MC_LOG_LEVEL_INFO}

struct mc_logger *mc_logger_new(struct mc_logger_config const *cfg);
void mc_logger_free(struct mc_logger *logger);
void mc_logger_set_level(struct mc_logger *logger, enum mc_log_level level);
enum mc_log_level mc_logger_get_level(struct mc_logger *logger);
void mc_logger_set_format(struct mc_logger *logger, int format_flags);
int mc_logger_get_format(struct mc_logger *logger);
void mc_logger_write(struct mc_logger *logger, enum mc_log_level level,
                     char const *file, int line, char const *fmt, ...);
void mc_logger_flush(struct mc_logger *logger);

#define MC_LOG_TRACE(logger, fmt, ...)                                         \
    mc_logger_write(logger, MC_LOG_LEVEL_TRACE, __FILE__, __LINE__, fmt,       \
                    ##__VA_ARGS__)

#define MC_LOG_DEBUG(logger, fmt, ...)                                         \
    mc_logger_write(logger, MC_LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt,       \
                    ##__VA_ARGS__)

#define MC_LOG_INFO(logger, fmt, ...)                                          \
    mc_logger_write(logger, MC_LOG_LEVEL_INFO, __FILE__, __LINE__, fmt,        \
                    ##__VA_ARGS__)

#define MC_LOG_WARN(logger, fmt, ...)                                          \
    mc_logger_write(logger, MC_LOG_LEVEL_WARN, __FILE__, __LINE__, fmt,        \
                    ##__VA_ARGS__)

#define MC_LOG_ERROR(logger, fmt, ...)                                         \
    mc_logger_write(logger, MC_LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt,       \
                    ##__VA_ARGS__)

#define MC_LOG_FATAL(logger, fmt, ...)                                         \
    do {                                                                       \
        mc_logger_write(logger, MC_LOG_LEVEL_FATAL, __FILE__, __LINE__, fmt,   \
                        ##__VA_ARGS__);                                        \
        mc_logger_flush(logger);                                               \
    } while (0)

#endif
