#ifndef __OS_TOOLS_H__
#define __OS_TOOLS_H__


#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef enum
{
    OS_LOG_SILENT = 0,
    OS_LOG_ERROR,
    OS_LOG_ERR = OS_LOG_ERROR,
    OS_LOG_WARNING,
    OS_LOG_WARN = OS_LOG_WARNING,
    OS_LOG_INFO,
    OS_LOG_DEBUG,
    OS_LOG_TRACE,
} OsLogPriority;

#ifdef __cplusplus
extern "C" {
#endif

int system_wrap(char *buf, size_t bufsz, const char *msg, ...);

#define SYSTEM(msg, ...) system_wrap((char *)__builtin_alloca(256), 256, msg, ##__VA_ARGS__)

int32_t os_set_epoch_time(time_t seconds);

time_t os_get_epoch_time();

uint64_t os_get_timestamp_ms();

char *os_logts_str(char *buffer, size_t buffer_len);

uint64_t os_logts_ms();

int os_log_write(int prio, const char *tag, const char *text);

int os_log_vprintf(int prio, const char *tag, const char *fmt, va_list ap);

int os_log_printf(int prio, const char *tag, const char *fmt, ...);

static inline const char *os_log_prio_label(int prio)
{
    const char *prio2str_e = "E";
    const char *prio2str_w = "W";
    const char *prio2str_i = "I";
    const char *prio2str_d = "D";
    const char *prio2str_v = "V";
    //
    const char *priostr = "S";
    if (prio >= OS_LOG_TRACE)
    {
        priostr = prio2str_v;
    } else if (prio >= OS_LOG_DEBUG)
    {
        priostr = prio2str_d;
    } else if (prio >= OS_LOG_INFO)
    {
        priostr = prio2str_i;
    } else if (prio >= OS_LOG_WARNING)
    {
        priostr = prio2str_w;
    } else if (prio >= OS_LOG_ERROR)
    {
        priostr = prio2str_e;
    }
    return priostr;
}

#define OS_LOG_TAG "os"

#define OS_PRINT(msg, ...) os_log_printf(OS_LOG_DEBUG, OS_LOG_TAG, msg, ##__VA_ARGS__)


#define OS_LOGV(msg, ...)                                         \
    os_log_printf(OS_LOG_TRACE,                                   \
                  OS_LOG_TAG,                                     \
                  "[%s,%llu] <%s,%d>" msg,                        \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGD(msg, ...)                                         \
    os_log_printf(OS_LOG_DEBUG,                                   \
                  OS_LOG_TAG,                                     \
                  "[%s,%llu] <%s,%d> " msg,                       \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGI(msg, ...)                                         \
    os_log_printf(OS_LOG_INFO,                                    \
                  OS_LOG_TAG,                                     \
                  "[%s,%llu] <%s,%d> " msg,                       \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGW(msg, ...)                                         \
    os_log_printf(OS_LOG_WARNING,                                 \
                  OS_LOG_TAG,                                     \
                  "[%s,%llu] <%s,%d> " msg,                       \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGE(msg, ...)                                         \
    os_log_printf(OS_LOG_ERROR,                                   \
                  OS_LOG_TAG,                                     \
                  "[%s,%llu] <%s,%d> " msg,                       \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)



int os_setup_backtrace();

int is_sudo(int dump);

#ifdef __cplusplus
}
#endif



#endif  // __OS_TOOLS_H__
