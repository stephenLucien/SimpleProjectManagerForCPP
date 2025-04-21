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

#define CPP_MALLOC(type, name, cnt)                                                              \
    size_t                                 name##_sz = sizeof(type) * (cnt);                     \
    std::unique_ptr<type, void (*)(type*)> name##_cpp_ptr(static_cast<type*>(malloc(name##_sz)), \
                                                          [](type* ptr)                          \
                                                          {                                      \
                                                              if (ptr)                           \
                                                              {                                  \
                                                                  free(ptr);                     \
                                                              }                                  \
                                                          });                                    \
    type*                                  name = name##_cpp_ptr.get()

#define CPP_CALLOC(type, name, cnt)                                                                      \
    size_t                                 name##_sz = sizeof(type) * (cnt);                             \
    std::unique_ptr<type, void (*)(type*)> name##_cpp_ptr(static_cast<type*>(calloc(cnt, sizeof(type))), \
                                                          [](type* ptr)                                  \
                                                          {                                              \
                                                              if (ptr)                                   \
                                                              {                                          \
                                                                  free(ptr);                             \
                                                              }                                          \
                                                          });                                            \
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


class CppFileOpen
{
   private:
    FILE* fd = NULL;

   public:
    virtual ~CppFileOpen()
    {
        close();
    }
    CppFileOpen() = default;
    CppFileOpen(const std::string& f, const std::string& m, bool isPipe = false)
    {
        open(f, m, isPipe);
    }
    int close()
    {
        if (fd)
        {
            fclose(fd);
            fd = NULL;
        }
        return 0;
    }
    int open(const std::string& f, const std::string& m, bool isPipe = false)
    {
        close();
        if (isPipe)
        {
            fd = popen(f.c_str(), m.c_str());
        } else
        {
            fd = fopen(f.c_str(), m.c_str());
        }
        return fd ? 0 : -1;
    }
    FILE* get()
    {
        return fd;
    }
};


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



#endif  // __CPPHELPER_OS_H__
