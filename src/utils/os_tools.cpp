#include "os_tools.h"

//
#include <cxxabi.h>
#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <ucontext.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

//
#include "utils/cstring_proc.h"
#include "utils/pthread_mutex.hpp"

//
static const int blocksz = 1024;
//
static char log_buffer[blocksz * 4];
//
static PthreadMutex log_buffer_mtx;

int system_wrap(char *buf, size_t bufsz, const char *msg, ...)
{
    int ret = -1;
    //
    va_list ap;
    va_start(ap, msg);
    //
    int wc = vsnprintf(buf, bufsz, msg, ap);
    if (wc > 0)
    {
        ret = system(buf);
    }
    va_end(ap);
    return ret;
}

int32_t os_set_epoch_time(time_t seconds)
{
    int ret = -1;

    //
#if 1
    struct timeval tv = {.tv_sec = seconds, .tv_usec = 0};
    //
    ret = settimeofday(&tv, NULL);
#else
    ret = stime(&seconds);
#endif

    if (ret)
    {
        os_log_printf(OS_LOG_ERR, "os", "settimeofday failed, ret=%d, %s", ret, strerror(errno));
    } else
    {
        os_log_printf(OS_LOG_INFO, "os", "settimeofday okay");
    }
    return ret;
}

time_t os_get_epoch_time()
{
    time_t curTime;
    time(&curTime);
    return curTime;
}

uint64_t os_get_timestamp_ms()
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    uint64_t ts;
    ts = (uint64_t)monotime.tv_sec * 1000 + monotime.tv_nsec / 1000 / 1000;
    return ts;
}



char *os_logts_str(char *buffer, size_t buffer_len)
{
    time_t curTime;
    time(&curTime);

    struct tm curTm;
    localtime_r(&curTime, &curTm);
    strftime(buffer, buffer_len, "%Y%m%d_%H%M%S", &curTm);
    return buffer;
}

uint64_t os_logts_ms()
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    uint64_t ts;
    ts = (uint64_t)monotime.tv_sec * 1000 + monotime.tv_nsec / 1000 / 1000;
    return ts;
}

int __attribute__((weak)) os_log_write_impl(int prio, const char *tag, const char *text, size_t text_len)
{
    static PthreadMutex logMtx;
    //
    PthreadMutex::Writelock lock(logMtx);
    //
    int ret = -1;
    //
    FILE *fd = stdout;
    //
    auto wc1 = fprintf(fd, "%s/%s\t", os_log_prio_label(prio), tag);
    //
    auto wc2 = fwrite(text, 1, text_len, fd);
    //
    auto wc3 = fprintf(fd, "\n");

    if (wc1 < 0 || wc2 < 0 || wc3 < 0)
    {
        ret = -1;
    } else
    {
        ret = wc1 + wc2 + wc3;
    }
    return ret;
}

int os_log_write(int prio, const char *tag, const char *text)
{
    int len = 0;
    //
    std::vector<std::string> lines;
    split_utf8_string(text, lines, blocksz);
    //
    for (auto &l : lines)
    {
        os_log_write_impl(prio, tag, l.c_str(), l.length());
        len += (int)l.length();
    }

    return len;
}


int os_log_vprintf(int prio, const char *tag, const char *fmt, va_list ap)
{
    PthreadMutex::Writelock lock(log_buffer_mtx);
    //
    int ret;
    //
    int offset = 0;
    //
    ret = vsnprintf(log_buffer + offset, sizeof(log_buffer) - offset, fmt, ap);
    if (ret <= 0)
    {
        return ret;
    }

    ret = os_log_write(prio, tag, log_buffer);

    return ret;
}


int os_log_printf(int prio, const char *tag, const char *fmt, ...)
{
    int ret;
    //
    va_list ap;
    va_start(ap, fmt);
    ret = os_log_vprintf(prio, tag, fmt, ap);
    va_end(ap);
    return ret;
}

static void dump_backtrace(int sig_num, siginfo_t *info, void *ucontext)
{
    void  *array[50];
    char **messages;
    int    size, i;
    //
    OS_PRINT("signum: %d (%s)", sig_num, strsignal(sig_num));

    size = backtrace(array, 50);

    messages = backtrace_symbols(array, size);

    /* skip first stack frame (points here) */
    for (i = 1; i < size && messages != NULL; ++i)
    {
        int status;
        //
        char *realname = abi::__cxa_demangle(messages[i], NULL, NULL, &status);
        OS_LOGE("[bt]: (%d) %s", i, realname ? realname : messages[i] ? messages[i] : "nil");
        free(realname);
    }

    free(messages);

    exit(EXIT_FAILURE);
}

static inline int setup_backtrace_signal_hdl(int sig_num)
{
    int ret = 0;
    //
    struct sigaction sigact;

    sigact.sa_sigaction = dump_backtrace;
    //
    sigact.sa_flags = SA_RESETHAND;

    if (sigaction(sig_num, &sigact, (struct sigaction *)NULL) != 0)
    {
        OS_PRINT("error setting signal handler for %d (%s)\n", sig_num, strsignal(sig_num));

        ret = -1;
    }
    return ret;
}

int os_setup_backtrace()
{
    int ret = 0;

    if (setup_backtrace_signal_hdl(SIGSEGV))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGBUS))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGFPE))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGABRT))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGILL))
    {
        ret = -1;
    }
    return ret;
}

void __attribute__((weak)) cleanup_on_exit_impl(int sig_num)
{
}

static void cleanup_on_exit(int sig_num, siginfo_t *info, void *ucontext)
{
    //
    OS_LOGI("signum: %d (%s)", sig_num, strsignal(sig_num));
    //
    cleanup_on_exit_impl(sig_num);

    exit(EXIT_SUCCESS);
}

static inline int setup_exit_signal_hdl(int sig_num)
{
    int ret = 0;
    //
    struct sigaction sigact;

    sigact.sa_sigaction = cleanup_on_exit;
    //
    sigact.sa_flags = SA_RESETHAND;

    if (sigaction(sig_num, &sigact, (struct sigaction *)NULL) != 0)
    {
        OS_PRINT("error setting signal handler for %d (%s)\n", sig_num, strsignal(sig_num));

        ret = -1;
    }
    return ret;
}

int os_setup_exit()
{
    int ret = 0;

    if (setup_exit_signal_hdl(SIGINT))
    {
        ret = -1;
    }
    if (setup_exit_signal_hdl(SIGKILL))
    {
        ret = -1;
    }
    return ret;
}
