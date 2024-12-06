#include "cstring_proc.h"

//
#include <string>
#include <vector>

//
#include "common.h"

int __attribute__((weak)) str2upper(char* str)
{
    int ret = -1;
    if (!str)
    {
        return ret;
    }
    ret = 0;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (isalpha(str[i]))
        {
            auto c = toupper(str[i]);
            if (c != str[i])
            {
                str[i] = c;
                ret++;
            }
        }
    }
    return ret;
}


int __attribute__((weak)) str2lower(char* str)
{
    int ret = -1;
    if (!str)
    {
        return ret;
    }
    ret = 0;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (isalpha(str[i]))
        {
            auto c = tolower(str[i]);
            if (c != str[i])
            {
                str[i] = c;
                ret++;
            }
        }
    }
    return ret;
}

int __attribute__((weak)) strcicmp(char const* a, char const* b)
{
    if (!a || !b)
    {
        return 1;
    }
    for (;; a++, b++)
    {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
        {
            return d;
        }
    }
}


int32_t hexString2byteArray(unsigned char* byteArray, size_t byteArrayLen, const char* hexString, size_t hexStringLen)
{
    size_t i;
    //
    for (i = 0; i < (hexStringLen / 2) && i < byteArrayLen; ++i)
    {
        if ((hexString[2 * i] <= '9') && (hexString[2 * i] >= '0'))
        {
            byteArray[i] = (hexString[2 * i] - '0') * 16;
        } else if ((hexString[2 * i] <= 'F') && (hexString[2 * i] >= 'A'))
        {
            byteArray[i] = (hexString[2 * i] - 'A' + 0x0a) * 16;
        } else if ((hexString[2 * i] <= 'f') && (hexString[2 * i] >= 'a'))
        {
            byteArray[i] = (hexString[2 * i] - 'a' + 0x0a) * 16;
        } else
        {
            return -1;
        }

        if ((hexString[2 * i + 1] <= '9') && (hexString[2 * i + 1] >= '0'))
        {
            byteArray[i] += (hexString[i * 2 + 1] - '0');
        } else if ((hexString[2 * i + 1] <= 'F') && (hexString[2 * i + 1] >= 'A'))
        {
            byteArray[i] += (hexString[i * 2 + 1] - 'A' + 0xA);
        } else if ((hexString[2 * i + 1] <= 'f') && (hexString[2 * i + 1] >= 'a'))
        {
            byteArray[i] += (hexString[i * 2 + 1] - 'a' + 0xA);
        } else
        {
            return -1;
        }
    }

    if (byteArrayLen > i)
    {
        byteArray[i] = '\0';
    }

    return i;
}


int32_t byteArray2hexString(char* hexString, size_t hexStringLen, const unsigned char* byteArray, size_t byteArrayLen, int upcase)
{
    size_t i;
    //
    unsigned char byte_low, byte_high;
    //
    const char A = upcase ? 'A' : 'a';
    //
    for (i = 0; i < byteArrayLen && i < hexStringLen / 2; i++)
    {
        byte_low  = (byteArray[i] >> 0) & 0x0f;
        byte_high = (byteArray[i] >> 4) & 0x0f;
        if (byte_high < 0x0a)
        {
            hexString[i * 2] = byte_high + 0x30;
        } else
        {
            hexString[i * 2] = byte_high - 0x0a + A;
        }
        if (byte_low < 0x0a)
        {
            hexString[i * 2 + 1] = byte_low + 0x30;
        } else
        {
            hexString[i * 2 + 1] = byte_low - 0x0a + A;
        }
    }
    if (hexStringLen > 2 * i)
    {
        hexString[2 * i] = '\0';
    }

    return i;
}


int32_t r_byteArray2hexString(char* hexString, size_t hexStringLen, const unsigned char* r_byteArray, size_t r_byteArrayLen, int upcase)
{
    size_t i;
    //
    unsigned char byte_low, byte_high;
    //
    const char A = upcase ? 'A' : 'a';
    //
    for (i = 0; i < r_byteArrayLen && i < hexStringLen / 2; i++)
    {
        byte_low  = (r_byteArray[r_byteArrayLen - 1 - i] >> 0) & 0x0f;
        byte_high = (r_byteArray[r_byteArrayLen - 1 - i] >> 4) & 0x0f;
        if (byte_high < 0x0a)
        {
            hexString[i * 2] = byte_high + 0x30;
        } else
        {
            hexString[i * 2] = byte_high - 0x0a + A;
        }
        if (byte_low < 0x0a)
        {
            hexString[i * 2 + 1] = byte_low + 0x30;
        } else
        {
            hexString[i * 2 + 1] = byte_low - 0x0a + A;
        }
    }
    if (hexStringLen > 2 * i)
    {
        hexString[2 * i] = '\0';
    }


    return i;
}

int verstr2bytes(std::string ver_str, std::vector<uint8_t>& ver_bytes)
{
    int ret = 0;
    //
    std::string digit;

    ver_bytes.clear();

    do
    {
        // printf("%s\n", ver_str.c_str());
        auto pos = ver_str.find('.', 0);
        if (pos != std::string::npos)
        {
            digit = ver_str.substr(0, pos);
        } else
        {
            digit = ver_str.substr(0);
        }
        if (digit.empty())
        {
            break;
        }

        int num = 0;
        try
        {
            // printf("%s\n", digit.c_str());
            num = std::stoi(digit);
        } catch (const std::exception& e)
        {
            // printf("err\n");
            ret = 1;
        }
        if (ret)
        {
            break;
        }
        ver_bytes.push_back(num);

        if (pos == std::string::npos)
        {
            break;
        }
        ver_str = ver_str.substr(pos + 1);
    } while (1);

    if (ret)
    {
        return ret;
    }

    return ver_bytes.size();
}

int str2tokens(char* str, std::vector<char*>& tokens, const char* delim)
{
    int ret = -1;
    if (!str)
    {
        return ret;
    }
    do
    {
        char* rest_str = str;
        char* token;
        //
        while ((token = strtok_r(rest_str, delim, &rest_str)))
        {
            tokens.push_back(token);
        }
        ret = 0;
    } while (0);
    return ret;
}

std::vector<std::string> str_split(const std::string& s, const std::string& delimiter)
{
    std::vector<char> buf(s.length() + 1);
    memcpy(buf.data(), s.c_str(), s.length() + 1);
    //
    std::vector<char*> tokens;
    str2tokens(buf.data(), tokens, delimiter.c_str());
    //
    std::vector<std::string> substrings;
    substrings.reserve(tokens.size());
    for (auto& e : tokens)
    {
        substrings.push_back(e);
    }
    return substrings;
}


int split_utf8_line(const std::string& line, std::vector<std::string>& lines, size_t max_line_sz)
{
    int linecnt = 0;
    //
    const int maxBytesForUTF8Seq = 4;
    //
    if (max_line_sz <= maxBytesForUTF8Seq)
    {
        return linecnt;
    }
    //
    int est_w = max_line_sz - maxBytesForUTF8Seq;
    //
    size_t loglen = line.length();
    //
    char buf[max_line_sz];
    //
    auto ptr     = line.c_str();
    auto ptr_end = ptr + loglen;
    //
    int left;
    while ((left = ptr_end - ptr) > 0)
    {
        int cc = left > (int)est_w ? (int)est_w : left;
        memcpy(buf, ptr, cc);
        ptr += cc;

        for (int i = 0; i < maxBytesForUTF8Seq - 1; ++i)
        {
            if (!is_utf8_seq_continue(ptr[0]))
            {
                break;
            }

            buf[cc++] = ptr[0];
            ++ptr;
        }

        buf[cc] = 0;
        //
        linecnt++;
        lines.emplace_back((char*)buf);
    }
    return linecnt;
}

void split_utf8_string(const std::string& content, std::vector<std::string>& lines, size_t max_line_sz)
{
    std::vector<char> buf(content.length() + 1);
    //
    memcpy(buf.data(), content.c_str(), content.length() + 1);
    //
    std::vector<char*> tmplines;
    //
    str2tokens(buf.data(), tmplines, "\n");
    //
    lines.clear();
    lines.reserve(tmplines.size());
    for (auto str : tmplines)
    {
        // lines.emplace_back(str);
        split_utf8_line(str, lines, max_line_sz);
    }
}
