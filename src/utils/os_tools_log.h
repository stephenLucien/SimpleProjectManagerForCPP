#ifndef OS_TOOLS_LOG_H
#define OS_TOOLS_LOG_H

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "os_tools_time.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline const char *os_get_filename(const char *path)
{
    if (!path)
    {
        return "";
    }
    // unix-like path
    const char *pos0 = strrchr(path, '/');
    if (pos0)
    {
        return pos0 + 1;
    }
    // win-path
    const char *pos1 = strrchr(path, '\\');
    if (pos1)
    {
        return pos1 + 1;
    }
    return path;
}

#ifndef __FILE_NAME__
    #define __FILE_NAME__ (os_get_filename(__FILE__))
#endif

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

int os_log_write_impl(int prio, const char *tag, const char *text, size_t text_len);

int os_log_write(int prio, const char *tag, const char *text);

int os_log_vprintf(int prio, const char *tag, const char *fmt, va_list ap);

int os_log_printf(int prio, const char *tag, const char *fmt, ...);

#define OS_LOG_TAG "os"

#define OS_PRINT(msg, ...) os_log_printf(OS_LOG_DEBUG, OS_LOG_TAG, msg, ##__VA_ARGS__)


#define OS_LOGV(msg, ...)                                         \
    os_log_printf(OS_LOG_TRACE,                                   \
                  OS_LOG_TAG,                                     \
                  "[%s,%" PRIu64 "] <%s,%d>" msg,                 \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGD(msg, ...)                                         \
    os_log_printf(OS_LOG_DEBUG,                                   \
                  OS_LOG_TAG,                                     \
                  "[%s,%" PRIu64 "] <%s,%d> " msg,                \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGI(msg, ...)                                         \
    os_log_printf(OS_LOG_INFO,                                    \
                  OS_LOG_TAG,                                     \
                  "[%s,%" PRIu64 "] <%s,%d> " msg,                \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGW(msg, ...)                                         \
    os_log_printf(OS_LOG_WARNING,                                 \
                  OS_LOG_TAG,                                     \
                  "[%s,%" PRIu64 "] <%s,%d> " msg,                \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


#define OS_LOGE(msg, ...)                                         \
    os_log_printf(OS_LOG_ERROR,                                   \
                  OS_LOG_TAG,                                     \
                  "[%s,%" PRIu64 "] <%s,%d> " msg,                \
                  os_logts_str((char *)__builtin_alloca(64), 64), \
                  os_logts_ms(),                                  \
                  __FUNCTION__,                                   \
                  __LINE__,                                       \
                  ##__VA_ARGS__)


const char *os_log_hexdump2buf(char *buf, size_t bufsz, void *data, int datalen, int print_addr, int bytes_per_pack, int packs_per_line);

#define OS_LOG_HEXDUMP(data, len) os_log_hexdump2buf((char *)__builtin_alloca(4096), 4096, (void *)data, len, 1, 4, 4)


#ifndef DEBUG
    #define DEBUG                                                     \
        os_log_printf(OS_LOG_DEBUG,                                   \
                      "dbg",                                          \
                      "[%s,%" PRIu64 "] <fn:%s,func:%s,lno:%d>",      \
                      os_logts_str((char *)__builtin_alloca(64), 64), \
                      os_logts_ms(),                                  \
                      __FILE_NAME__,                                  \
                      __FUNCTION__,                                   \
                      __LINE__);
#endif

#ifdef __cplusplus
}
#endif

#endif /* OS_TOOLS_LOG_H */
