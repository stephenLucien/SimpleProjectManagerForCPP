#include "os_tools_log.h"

//
#include "cstring_proc.h"
#include "pthread_mutex.hpp"

//
#include <string.h>
#include <time.h>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
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

const char *os_log_hexdump2buf(char *buf, size_t bufsz, void *data, int datalen, int print_addr, int bytes_per_pack, int packs_per_line)
{
    if (!buf || bufsz <= 0)
    {
        return buf;
    }
    memset(buf, 0, bufsz);
    if (!data || datalen <= 0)
    {
        return buf;
    }
    if (bytes_per_pack <= 0)
    {
        bytes_per_pack = 4;
    }
    if (packs_per_line <= 0)
    {
        packs_per_line = 4;
    }
    int bytes_per_line = bytes_per_pack * packs_per_line;
    //
    int wc = 0;
    //
    size_t offset = 0;
    //
    auto ptr = (uint8_t *)data;
    //
    for (int i = 0; i < datalen && offset < bufsz; ++i)
    {
        if (i % bytes_per_pack == 0)
        {
            if (i % bytes_per_line == 0)
            {
                wc = snprintf(buf + offset, bufsz - offset, "\n");
                if (wc < 0)
                {
                    break;
                }
                offset += wc;

                if (print_addr)
                {
                    wc = snprintf(buf + offset, bufsz - offset, "[%p] ", ptr + i);
                    if (wc < 0)
                    {
                        break;
                    }
                    offset += wc;
                }
            } else
            {
                wc = snprintf(buf + offset, bufsz - offset, " ");
                if (wc < 0)
                {
                    break;
                }
                offset += wc;
            }
        }
        wc = snprintf(buf + offset, bufsz - offset, "%02X", ptr[i]);
        if (wc < 0)
        {
            break;
        }
        offset += wc;
    }
    return buf;
}
