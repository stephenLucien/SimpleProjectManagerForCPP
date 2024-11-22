#include "os_tools_log.h"

//
#include "cstring_proc.h"
#include "pthread_mutex.hpp"

//
#include <string.h>
#include <time.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>


//
static const int blocksz = 1024;
//
static char log_buffer[blocksz * 4];
//
static PthreadMutex log_buffer_mtx;


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
