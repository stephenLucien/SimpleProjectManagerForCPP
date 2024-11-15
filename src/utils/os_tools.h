#ifndef __OS_TOOLS_H__
#define __OS_TOOLS_H__


#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

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


#define OS_LOG_TAG "os"

#define OS_PRINT(msg, ...) \
    os_log_printf(0, OS_LOG_TAG, "[%s,%llu] " msg, os_logts_str((char *)__builtin_alloca(64), 64), os_logts_ms(), ##__VA_ARGS__)


int os_setup_backtrace();

#ifdef __cplusplus
}
#endif



#endif  // __OS_TOOLS_H__
