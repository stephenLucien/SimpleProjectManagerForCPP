#include "os_tools_system.h"

//
#include "os_tools_log.h"

//
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

//
#include "cpp_helper/cpphelper_os.hpp"

//
int m_app_running = 1;
//
static int m_return_code = EXIT_SUCCESS;



void __attribute__((weak)) os_init_on_startup_impl(int num)
{
    OS_LOGV("%s", __FILE__);
}


void __attribute__((weak)) os_deinit_on_exit_impl(int sig_num)
{
    OS_LOGV("%s", __FILE__);
}

//
void __attribute__((weak)) os_running_loop_impl()
{
    OS_LOGV("%s", __FILE__);
    while (m_app_running)
    {
        sleep(1);
    }
    OS_LOGV("%s", __FILE__);
}

void __attribute__((weak)) os_stop_running_loop_impl(int code)
{
    OS_LOGV("%s", __FILE__);
}


void os_init_on_startup(int num)
{
    //
    os_setup_backtrace();
    os_setup_exit();

    //
    os_init_on_startup_impl(num);
}

int os_running_loop()
{
    //
    os_running_loop_impl();
    //
    os_deinit_on_exit_impl(0);

    return m_return_code;
}


void os_stop_running_loop(int code)
{
    os_stop_running_loop_impl(code);
    //
    m_return_code = code;
    //
    m_app_running = 0;
}


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

int is_sudo(int dump)
{
    auto uid  = getuid();
    auto euid = geteuid();

    if (uid == 0)
    {
        if (dump)
        {
            OS_LOGV("root");
        }
        return 1;
    }
    if (euid == 0)
    {
        if (dump)
        {
            OS_LOGV("sudo");
        }
        return 1;
    }
    if (dump)
    {
        OS_LOGV("uid: %u", uid);
        OS_LOGV("euid: %u", euid);
    }
    return 0;
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

    //
    os_stop_running_loop(EXIT_FAILURE);
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


static void on_exit_signal(int sig_num, siginfo_t *info, void *ucontext)
{
    //
    OS_LOGW("signum: %d (%s)", sig_num, strsignal(sig_num));

    //
    os_stop_running_loop(EXIT_SUCCESS);
}

static inline int setup_exit_signal_hdl(int sig_num)
{
    int ret = 0;
    //
    struct sigaction sigact;

    sigact.sa_sigaction = on_exit_signal;
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
#if 0
    // Will always fail, SIGKILL is intended to force kill your process
    if (setup_exit_signal_hdl(SIGKILL))
    {
        // You can never handle SIGKILL anyway...
        ret = -1;
    }
#endif
    return ret;
}


int write_data_to_file(const char *fn, const char *data, size_t datalen)
{
    int ret = -1;
    //
    if (!data || datalen <= 0)
    {
        return ret;
    }
    //
    CPP_FOPEN(fd, fn, "w");
    if (!fd)
    {
        OS_PRINT("open file fail: %s", fn);
        return ret;
    }
    size_t wc = fwrite(data, 1, datalen, fd);
    if (wc != datalen)
    {
        OS_PRINT("write len(%zu/%zu) %s", wc, datalen, fn);
    }

    return (int)wc;
}

char *read_data_from_file(const char *fn, char *buf, size_t bufsz, int *read_cnt)
{
    if (read_cnt)
    {
        *read_cnt = -1;
    }
    if (!buf || bufsz <= 0)
    {
        return buf;
    }
    //
    memset(buf, 0, bufsz);
    //
    CPP_FOPEN(fd, fn, "r");
    if (!fd)
    {
        OS_PRINT("open file fail: %s", fn);
        return buf;
    }
    do
    {
        if (fseek(fd, 0, SEEK_END) != 0)
        {
            OS_PRINT("fseek end fail:%s", fn);
            break;
        }
        auto sz = ftell(fd);
        if (sz < 0)
        {
            OS_PRINT("ftell fail:%s", fn);
            break;
        }
        if (fseek(fd, 0, SEEK_SET) != 0)
        {
            OS_PRINT("fseek set fail:%s", fn);
            break;
        }
        if (sz >= bufsz)
        {
            OS_PRINT("overflow, fn:%s, max:%zu, but %ld ", fn, bufsz, sz);
            break;
        }

        auto rc = fread(buf, 1, sz, fd);
        if (read_cnt)
        {
            *read_cnt = (int)rc;
        }
        if (rc == sz)
        {
        } else
        {
            OS_PRINT("read incomplete (%zu, %d): %s", rc, sz, fn);
        }

    } while (0);

    return buf;
}
