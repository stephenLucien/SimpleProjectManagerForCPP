#ifndef __CSTRING_PROC_H__
#define __CSTRING_PROC_H__

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int str_without_unprint(const char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        if (!isprint(str[i]))
        {
            return 0;
        }
    }
    return 1;
}

static inline bool is_utf8_seq_start(char c)
{
    return (c & 0b11000000) == 0b11000000;
}

static inline bool is_utf8_seq_continue(char c)
{
    return (c & 0b11000000) == 0b10000000;
}


/**
 * @brief
 *
 * @param str
 * @return int
 * - negative : error
 * - other : the number of characters has changed.
 */
int str2upper(char* str);

/**
 * @brief
 *
 * @param str
 * @return int
 * - negative : error
 * - other : the number of characters has changed.
 */
int str2lower(char* str);

/**
 * @brief case-insensitive strcmp
 *
 * @param a
 * @param b
 * @return int
 * - 0 : same str
 * - other : diff
 */
int strcicmp(char const* a, char const* b);

/**
 * @brief convert hex string to byte array
 *
 * @param [out] byteArray
 * @param [in] byteArrayLen
 * @param [in] hexString
 * @param [in] hexStringLen
 * @return int32_t
 * - negative : err
 * - other : number of bytes being converted
 */
int32_t hexString2byteArray(unsigned char* byteArray, size_t byteArrayLen, const char* hexString, size_t hexStringLen);

/**
 * @brief convert byte array to hex string
 *
 * @param [out] hexString
 * @param [in] hexStringLen
 * @param [in] byteArray
 * @param [in] byteArrayLen
 * @return int32_t
 * - negative : err
 * - other : number of bytes being converted
 */
int32_t byteArray2hexString(char* hexString, size_t hexStringLen, const unsigned char* byteArray, size_t byteArrayLen, int upcase = 1);

int32_t r_byteArray2hexString(char* hexString, size_t hexStringLen, const unsigned char* r_byteArray, size_t r_byteArrayLen, int upcase = 1);

int revert_array_inplace(void* data, size_t datasz, int step, void* swap_buf);

static inline void revert_byte_array(void* data, size_t len)
{
    uint8_t swap_buf;
    revert_array_inplace(data, len, 1, &swap_buf);
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
    #include <string>
    #include <vector>

/**
 * @brief convert version string to bytes
 * example: "1.2.3" --> {1,2,3}
 * @param [in] ver_str : "1.2.3"
 * @param [out] ver_bytes : {1,2,3}
 * @return int
 * - negative : err
 * - 0 : empty
 * - positive : bytes converted.
 */
int verstr2bytes(std::string ver_str, std::vector<uint8_t>& ver_bytes);

int str2tokens(char* str, std::vector<char*>& tokens, const char* delim = ",");

std::vector<std::string> str_split(const std::string& s, const std::string& delimiter);

/**
 * @brief
 * @note \ref https://en.wikipedia.org/wiki/UTF-8
 *
 * @param line
 * @param lines
 * @param max_line_sz
 * @return int
 */
int split_utf8_line(const std::string& line, std::vector<std::string>& lines, size_t max_line_sz);

//
void split_utf8_string(const std::string& content, std::vector<std::string>& lines, size_t max_line_sz);


#endif

#endif  // __CSTRING_PROC_H__
