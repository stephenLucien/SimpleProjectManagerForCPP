#ifndef OS_TOOLS_SYSTEM_H
#define OS_TOOLS_SYSTEM_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int system_wrap(char *buf, size_t bufsz, const char *msg, ...);

#define SYSTEM(msg, ...) system_wrap((char *)__builtin_alloca(256), 256, msg, ##__VA_ARGS__)

int is_sudo(int dump = 0);

int os_setup_backtrace();

void cleanup_on_exit_impl(int sig_num);

int os_setup_exit();

#ifdef __cplusplus
}
#endif


#endif /* OS_TOOLS_SYSTEM_H */
