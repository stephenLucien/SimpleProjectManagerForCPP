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
