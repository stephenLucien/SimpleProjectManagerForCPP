#include "os_tools_log.h"

//
#include "pthread_mutex.hpp"

//
#include <stdio.h>
#include <string.h>
#include <cstddef>

//
static FILE *m_orig_stdout = NULL;
static FILE *m_orig_stderr = NULL;

//
static PthreadMutex m_log_fd_mtx;
//
static char logfile[256] = "program.log";
//
static FILE *m_log_fd = NULL;
//
const char *os_log_setup_log_file(const char *path)
{
    PthreadMutex::Writelock lock(m_log_fd_mtx);
    if (!path)
    {
        return path;
    }
    snprintf(logfile, sizeof(logfile), "%s", path);
    return logfile;
}


void os_tools_close_log_file()
{
    PthreadMutex::Writelock lock(m_log_fd_mtx);
    if (m_log_fd)
    {
        //
        fprintf(stderr, "\n\r");
        fprintf(stdout, "Stop logging to %s......\n", logfile);
        fclose(m_log_fd);
        m_log_fd = NULL;
    }
}

int os_tools_open_log_file()
{
    //
    PthreadMutex::Writelock lock(m_log_fd_mtx);
    if (m_log_fd)
    {
        return 0;
    }
    if (strlen(logfile) <= 0)
    {
        return -1;
    }
    /**
     * @note: write logs to file
     *
     */
    m_log_fd = fopen(logfile, "w");
    if (!m_log_fd)
    {
        fprintf(stderr, "failed to open logfile for write:%s \n\r", logfile);
        return -1;
    }
    //
    fprintf(stderr, "\n\r");
    fprintf(stdout, "Start logging to %s......\n", logfile);

    return 0;
}

int os_tools_reopen_log_file()
{
    os_tools_close_log_file();
    return os_tools_open_log_file();
}

void os_tools_flush_log_file()
{
    PthreadMutex::Writelock lock(m_log_fd_mtx);
    if (m_log_fd)
    {
        fflush(m_log_fd);
    }
}

//
int os_log_write_impl(int prio, const char *tag, const char *text, size_t text_len)
{
    //
    PthreadMutex::Writelock lock(m_log_fd_mtx);
    //
    int ret = -1;
    //
    FILE *fd = m_orig_stdout;
    //
    if (m_log_fd)
    {
        fd = m_log_fd;
    }
    if (!fd)
    {
        return ret;
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


static __ssize_t cookie_read(void *__cookie, char *__buf, size_t __nbytes)
{
    return -1;
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
    return -1;
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
    //
    auto tmpstream = fopencookie((void *)"stdout", "w", log_fns);
    if (tmpstream)
    {
        setvbuf(tmpstream, NULL, _IOLBF, 0);
        fprintf(stdout, "stdout: %p(%p) --> %p\n", stdout, m_orig_stdout, tmpstream);
        fflush(stdout);
        if (m_orig_stdout == NULL)
        {
            m_orig_stdout = stdout;
        }
        //
        stdout = tmpstream;
    }
}

void stderr2log()
{
    auto tmpstream = fopencookie((void *)"stderr", "w", log_fns);
    if (tmpstream)
    {
        setvbuf(tmpstream, NULL, _IOLBF, 0);
        fprintf(stderr, "stderr: %p(%p) --> %p\n", stderr, m_orig_stderr, tmpstream);
        fflush(stderr);
        if (m_orig_stderr == NULL)
        {
            m_orig_stderr = stderr;
        }
        //
        stderr = tmpstream;
    }
}

void restore_stdout()
{
    if (m_orig_stdout && stdout != m_orig_stdout)
    {
        auto tmp = stdout;
        //
        fprintf(stdout, "stdout: %p --> %p\n", stdout, m_orig_stdout);
        fflush(stdout);
        //
        stdout = m_orig_stdout;
        //
        fprintf(stdout, "close %p\n", tmp);
        fflush(stdout);
        //
        fclose(tmp);
    }
}

void restore_stderr()
{
    if (m_orig_stderr && stderr != m_orig_stderr)
    {
        auto tmp = stderr;
        //
        fprintf(stderr, "stderr: %p --> %p\n", stderr, m_orig_stderr);
        fflush(stderr);
        //
        stderr = m_orig_stderr;
        //
        fprintf(stderr, "close %p\n", tmp);
        fflush(stderr);
        //
        fclose(tmp);
    }
}


__attribute__((constructor)) static void setup_log_redirect()
{
    m_orig_stdout = stdout;
    m_orig_stderr = stderr;

    //
    os_tools_open_log_file();

    //
    stdout2log();
    stderr2log();
}

__attribute__((destructor)) static void unset_log_redirect()
{
    restore_stdout();
    restore_stderr();
    //
    os_tools_close_log_file();
}

#if 1
    #include <signal.h>
    #include "manager/cleanup_manager.h"
int flush_oslog(int reason, void *userdata)
{
    //
    os_tools_flush_log_file();
    //
    if (reason == SIGKILL || reason == SIGINT)
    {
        // unset_log_redirect();
    }
    return 0;
}

REG_CLEANUP_FUNC(FlushLog, flush_oslog, NULL)
#endif
