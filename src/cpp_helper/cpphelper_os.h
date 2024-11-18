#ifndef __CPPHELPER_OS_H__
#define __CPPHELPER_OS_H__

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//
#include <cstdint>
#include <memory>
#include <string>

//
#include "utils/os_tools.h"

#define CPP_MALLOC(type, name, cnt)                                                                       \
    std::unique_ptr<type, void (*)(type*)> name##_cpp_ptr(static_cast<type*>(malloc(sizeof(type) * cnt)), \
                                                          [](type* ptr)                                   \
                                                          {                                               \
                                                              if (ptr)                                    \
                                                              {                                           \
                                                                  free(ptr);                              \
                                                              }                                           \
                                                          });                                             \
    type*                                  name = name##_cpp_ptr.get()


#define CPP_FOPEN(fd, path, mode)                                            \
    std::unique_ptr<FILE, void (*)(FILE*)> fd##_cpp_ptr(fopen(path, mode),   \
                                                        [](FILE* ptr)        \
                                                        {                    \
                                                            if (ptr)         \
                                                            {                \
                                                                fclose(ptr); \
                                                            }                \
                                                        });                  \
    FILE*                                  fd = fd##_cpp_ptr.get()

#define CPP_POPEN(fd, cmd, mode)                                             \
    std::unique_ptr<FILE, void (*)(FILE*)> fd##_cpp_ptr(popen(cmd, mode),    \
                                                        [](FILE* ptr)        \
                                                        {                    \
                                                            if (ptr)         \
                                                            {                \
                                                                fclose(ptr); \
                                                            }                \
                                                        });                  \
    FILE*                                  fd = fd##_cpp_ptr.get()

#define CPP_OPENDIR(dir, path)                                                \
    std::unique_ptr<DIR, void (*)(DIR*)> dir##_cpp_ptr(opendir(path),         \
                                                       [](DIR* ptr)           \
                                                       {                      \
                                                           if (ptr)           \
                                                           {                  \
                                                               closedir(ptr); \
                                                           }                  \
                                                       });                    \
    DIR*                                 dir = dir##_cpp_ptr.get()



static inline int write_data_to_file(const std::string& filename, const char* data, size_t datalen)
{
    int ret = -1;
    //
    auto fn = filename.c_str();
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

static inline char* read_data_from_file(const std::string& filename, char* buf, size_t bufsz)
{
    if (!buf)
    {
        return buf;
    }
    //
    memset(buf, 0, bufsz);
    //
    auto fn = filename.c_str();
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
        if (rc == sz)
        {
        } else
        {
            OS_PRINT("read fail: %s", fn);
        }

    } while (0);


    return buf;
}

#endif  // __CPPHELPER_OS_H__
