#ifndef __CPPHELPER_DEBUG_H__
#define __CPPHELPER_DEBUG_H__

#include <inttypes.h>
#include <stdint.h>

// \ref https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
#define SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix, surfix, pst, field)                                                                 \
    ({                                                                                                                                           \
        int ret = 0;                                                                                                                             \
        if (0)                                                                                                                                   \
        {                                                                                                                                        \
        } else if (typeid(((pst)->field)) == typeid(int8_t) || typeid(((pst)->field)) == typeid(char))                                           \
        { /* char or signed char */                                                                                                              \
            auto tmp_ptr = ((const int8_t*)(&((pst)->field)));                                                                                   \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRId8 "(0x%02X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);   \
        } else if (typeid(((pst)->field)) == typeid(uint8_t))                                                                                    \
        { /* unsigned char */                                                                                                                    \
            auto tmp_ptr = ((const uint8_t*)(&((pst)->field)));                                                                                  \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRIu8 "(0x%02X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);   \
        } else if (typeid(((pst)->field)) == typeid(int16_t))                                                                                    \
        { /* signed short */                                                                                                                     \
            auto tmp_ptr = ((const int16_t*)(&((pst)->field)));                                                                                  \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRId16 "(0x%04X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);  \
        } else if (typeid(((pst)->field)) == typeid(uint16_t))                                                                                   \
        { /* unsigned short */                                                                                                                   \
            auto tmp_ptr = ((const uint16_t*)(&((pst)->field)));                                                                                 \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRIu16 "(0x%04X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);  \
        } else if (typeid(((pst)->field)) == typeid(int32_t))                                                                                    \
        {                                                                                                                                        \
            auto tmp_ptr = ((const int32_t*)(&((pst)->field)));                                                                                  \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRId32 "(0x%08X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);  \
        } else if (typeid(((pst)->field)) == typeid(uint32_t))                                                                                   \
        {                                                                                                                                        \
            auto tmp_ptr = ((const uint32_t*)(&((pst)->field)));                                                                                 \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRIu32 "(0x%08X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);  \
        } else if (typeid(((pst)->field)) == typeid(int64_t))                                                                                    \
        {                                                                                                                                        \
            auto tmp_ptr = ((const int64_t*)(&((pst)->field)));                                                                                  \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRId64 "(0x%016X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix); \
        } else if (typeid(((pst)->field)) == typeid(uint64_t))                                                                                   \
        {                                                                                                                                        \
            auto tmp_ptr = ((const uint64_t*)(&((pst)->field)));                                                                                 \
            ret = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%" PRIu64 "(0x%016X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix); \
        } else if (typeid(((pst)->field)) == typeid(signed long long))                                                                           \
        {                                                                                                                                        \
            auto tmp_ptr = ((const signed long long*)(&((pst)->field)));                                                                         \
            ret          = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%lld(0x%X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);  \
        } else if (typeid(((pst)->field)) == typeid(unsigned long long))                                                                         \
        {                                                                                                                                        \
            auto tmp_ptr = ((const unsigned long long*)(&((pst)->field)));                                                                       \
            ret          = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%llu(0x%X)%s", prefix, (*tmp_ptr), (*tmp_ptr), surfix);  \
        } else if (typeid(((pst)->field)) == typeid(bool))                                                                                       \
        {                                                                                                                                        \
            auto tmp_ptr = ((const bool*)(&((pst)->field)));                                                                                     \
            ret          = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%s%s", prefix, (*tmp_ptr) ? "true" : "false", surfix);   \
        } else if (typeid(((pst)->field)) == typeid(std::string) || typeid(((pst)->field)) == typeid(const std::string))                         \
        {                                                                                                                                        \
            auto tmp_ptr = ((const std::string*)(&((pst)->field)));                                                                              \
            ret          = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%s%s", prefix, (*tmp_ptr).c_str(), surfix);              \
        } else if (typeid(((pst)->field)) == typeid(char*) || typeid(((pst)->field)) == typeid(const char*))                                     \
        { /* string pointer */                                                                                                                   \
            auto tmp_ptr = ((const char* const*)(&((pst)->field)));                                                                              \
            ret          = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%s%s", prefix, (*tmp_ptr), surfix);                      \
        } else if (strstr(typeid(((pst)->field)).name(), "_c"))                                                                                  \
        { /* char array: \ref https://stackoverflow.com/questions/2528318/how-come-an-arrays-address-is-equal-to-its-value-in-c */               \
            /* (pst)->field --> refer to address of first element; (pst)->field + 1 --> address of next element */                               \
            /* &((pst)->field) --> refer to address of array; &((pst)->field) + 1 --> address of next array */                                   \
            /* (pst)->field == &((pst)->field) , but they are different at step size */                                                          \
            auto tmp_ptr = ((const char*)(&((pst)->field)));                                                                                     \
            ret          = snprintf((buf) + (offset), (bufsz) - (offset), "%s" #field "=%s%s", prefix, tmp_ptr, surfix);                         \
        }                                                                                                                                        \
        if (ret >= 0)                                                                                                                            \
        {                                                                                                                                        \
            offset += ret;                                                                                                                       \
        }                                                                                                                                        \
        ret;                                                                                                                                     \
    })

#define PRINTF_STRUCTURE_FIELD(pfunc, bufsz, pst, field)                        \
    do                                                                          \
    {                                                                           \
        char buf[bufsz];                                                        \
        int  offset = 0;                                                        \
        memset(buf, 0, sizeof(buf));                                            \
        SNPRINTF_STRUCTURE_FIELD(buf, bufsz - 1, offset, "", "\n", pst, field); \
        pfunc("%s", buf);                                                       \
    } while (0)

#endif  // __CPPHELPER_DEBUG_H__
