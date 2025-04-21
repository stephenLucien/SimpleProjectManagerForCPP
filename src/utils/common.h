#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef FREE
    #define FREE(ptr)         \
        do                    \
        {                     \
            if (ptr)          \
            {                 \
                free(ptr);    \
                (ptr) = NULL; \
            }                 \
        } while (0)
#endif

#ifndef ARRSZ
    #define ARRSZ(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#ifndef CP2STBUF
    #define CP2STBUF(stbuf, data, datalen)                                           \
        do                                                                           \
        {                                                                            \
            size_t bufsize = sizeof(stbuf);                                          \
            memset((stbuf), 0, (bufsize));                                           \
            memcpy((stbuf), (data), (datalen) <= (bufsize) ? (datalen) : (bufsize)); \
        } while (0)
#endif


#endif  // __COMMON_H__
