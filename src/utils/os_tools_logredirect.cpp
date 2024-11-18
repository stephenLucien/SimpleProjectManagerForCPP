#include "os_tools.h"
#include "utils/pthread_mutex.hpp"

//
#include <stdio.h>
#include <string.h>
//
int os_log_write_impl(int prio, const char *tag, const char *text, size_t text_len);


static __ssize_t cookie_read(void *__cookie, char *__buf, size_t __nbytes)
{
    // return -1;
    return 0;
}

static __ssize_t cookie_write(void *__cookie, const char *__buf, size_t __nbytes)
{
    const char *tagstr = (const char *)__cookie;
    //
    int prio = OS_LOG_DEBUG;
    if (tagstr && strncmp(tagstr, "stderr", strlen("stderr")) == 0)
    {
        prio = OS_LOG_ERR;
    }

    return os_log_write_impl(prio, tagstr ? tagstr : OS_LOG_TAG, __buf, __nbytes);
}

static int cookie_seek(void *__cookie, __off64_t *__pos, int __w)
{
    // return -1;
    return 0;
}

static int cookie_close(void *__cookie)
{
    return 0;
}

static cookie_io_functions_t log_fns = {(cookie_read_function_t *)cookie_read,
                                        (cookie_write_function_t *)cookie_write,
                                        (cookie_seek_function_t *)cookie_seek,
                                        (cookie_close_function_t *)cookie_close};

void stdout2log()
{
    stdout = fopencookie((void *)"stdout", "w", log_fns);
    setvbuf(stdout, NULL, _IOLBF, 0);
}

void stderr2log()
{
    stderr = fopencookie((void *)"stderr", "w", log_fns);
    setvbuf(stderr, NULL, _IOLBF, 0);
}

#if 0
    #include <syslog.h>

int os_log_write_impl(int prio, const char *tag, const char *text, size_t text_len)
{
    openlog(tag, LOG_PID, LOG_USER);
    syslog(LOG_DEBUG, "%s", text);
    closelog();
    return 0;
}

__attribute__((constructor)) static void set_syslog()
{
    stdout2log();
    stderr2log();
    // openlog("Logs", LOG_PID, LOG_USER);
    // syslog(LOG_INFO, "Start logging ......");
}

__attribute__((destructor)) static void unset_syslog()
{
    // closelog();
}
#else

static FILE *m_log_fd = NULL;
//
int os_log_write_impl(int prio, const char *tag, const char *text, size_t text_len)
{
    static PthreadMutex logMtx;
    //
    PthreadMutex::Writelock lock(logMtx);
    //
    int ret = -1;
    //
    FILE *fd = stdout;
    //
    if (m_log_fd)
    {
        fd = m_log_fd;
    }
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

__attribute__((constructor)) static void setup_log_redirect()
{
    stdout2log();
    stderr2log();
    /**
     * @note: write logs to file
     *
     */
    m_log_fd = fopen("program.log", "w");

    //
    fprintf(stderr, "\n\r");
    fprintf(stdout, "Start logging ......\n");
}

__attribute__((destructor)) static void unset_log_redirect()
{
    if (m_log_fd)
    {
        fclose(m_log_fd);
        m_log_fd = NULL;
    }
}

#endif
