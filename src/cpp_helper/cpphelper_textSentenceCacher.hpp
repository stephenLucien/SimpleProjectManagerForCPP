#ifndef __CPPHELPER_TEXTSENTENCECACHER_H__
#define __CPPHELPER_TEXTSENTENCECACHER_H__


#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
#include "utils/os_tools_log.h"


#define SENTENCE_CACHE_ALL_STRING 1

class TextSentenceCacher
{
   private:
    std::unordered_set<std::string> wchar_delimiters = {
        /* ansi char */
        ",",
        ";",
        ".",
        "!",
        "?",
        ":",
        /* chinese char */
        "，",
        "；",
        "。",
        "！",
        "？",
        "：",
    };
    //
    std::string leftString;
#if defined(SENTENCE_CACHE_ALL_STRING) && SENTENCE_CACHE_ALL_STRING != 0
    //
    std::string allString;
#endif

   public:
    TextSentenceCacher()
    {
        clear();
    }
    void clear()
    {
        leftString.clear();
#if defined(SENTENCE_CACHE_ALL_STRING) && SENTENCE_CACHE_ALL_STRING != 0
        allString.clear();
#endif
        if (0)
        {
            for (auto &del : wchar_delimiters)
            {
                OS_LOGV("%s", del.c_str());
            }
        }
    }
    void append(const std::string &txt)
    {
        //
        leftString += txt;

#if defined(SENTENCE_CACHE_ALL_STRING) && SENTENCE_CACHE_ALL_STRING != 0
        allString += txt;
#endif
    }

    static const char *widecharString(char *buf, size_t bufsz, uint32_t c)
    {
        memset(buf, 0, bufsz);
        if (bufsz <= sizeof(c))
        {
            return buf;
        }
        memcpy(buf, &c, sizeof(c));
        return buf;
    }

#if defined(SENTENCE_CACHE_ALL_STRING) && SENTENCE_CACHE_ALL_STRING != 0
    const char *getAllString()
    {
        return allString.c_str();
    }
#endif


    const char *getLeftString()
    {
        return leftString.c_str();
    }

    size_t getLeftLength()
    {
        return leftString.length();
    }

    size_t getLeftCharCnt()
    {
        return utf8_length(leftString.c_str(), leftString.length());
    }

    std::string popSentenceImpl(size_t limit_min = 8, size_t limit_max = 300)
    {
        std::string s;
        //
        auto cnt = utf8_length(leftString.c_str(), leftString.length());
        //
        if (cnt < limit_min)
        {
            return s;
        }
        //
        if (cnt <= limit_max)
        {
            s = maxSentence();
            //
            auto cnt = utf8_length(s.c_str(), s.length());
            if (cnt < limit_min)
            {
                s.clear();
            } else
            {
                //
                popFront(s.length());
            }
            return s;
        }
        s = minSentence();
        if (utf8_length(s.c_str(), s.length()) > limit_max)
        {
            s = popFrontSentence(limit_max);
            return s;
        }
        //
        popFront(s.length());
        //
        while (leftString.length() > 0)
        {
            auto t = minSentence();
            if (t.empty())
            {
                break;
            }
            //
            auto tmps = s + t;
            if (utf8_length(tmps.c_str(), tmps.length()) > limit_max)
            {
                break;
            }
            s = tmps;
            //
            popFront(t.length());
        }

        return s;
    }

    std::string popSentence(int flush_flag = 0, size_t limit_min = 8, size_t limit_max = 300)
    {
        std::string s;
        // no data
        if (getLeftLength() <= 0)
        {
            return s;
        }
        // flush
        if (flush_flag)
        {
            if (getLeftCharCnt() <= limit_max)
            {
                s = getLeftString();
                clear();
            } else
            {
                s = popSentenceImpl(0, limit_max);
                if (s.empty())
                {
                    s = popFrontSentence(limit_max);
                }
            }
        } else
        {
            s = popSentenceImpl(limit_min, limit_max);
        }

        return s;
    }

    std::string popFrontSentence(size_t limit_max)
    {
        std::string s;
        //
        s = getFrontSentence(limit_max);
        //
        popFront(s.length());
        return s;
    }

    std::string popMinSentence()
    {
        auto s = minSentence();
        //
        popFront(s.length());
        return s;
    }

    std::string popMaxSentence()
    {
        auto s = maxSentence();
        //
        popFront(s.length());
        return s;
    }

    void popFront(size_t len)
    {
        if (len >= leftString.length())
        {
            leftString.clear();
        } else
        {
            leftString = leftString.substr(len);
        }
    }

    std::string getFrontSentence(size_t limit_max)
    {
        std::string s;
        //
        std::vector<char> buf((limit_max + 1) * 4);
        memset(buf.data(), 0, buf.size());
        utf8_strncpy(buf.data(), buf.size() - 1, s.c_str(), s.length(), limit_max);
        s = buf.data();
        return s;
    }

    // scan from begining
    std::string minSentence()
    {
        std::string s;
        if (leftString.empty())
        {
            return s;
        }
        auto pos = leftString.length();
        //
        auto ptr_e = leftString.data() + pos;
        auto ptr_b = leftString.data();
        //
        char buf[8];
        //
        const char *ptr = NULL;
        //
        size_t   csz = 0;
        uint32_t utf8_c;
        //
        bool b_found_del = false;

        ptr = ptr_b;
        //
        while (ptr < ptr_e)
        {
            utf8_c = 0;
            //
            csz = next_utf8_char(ptr, ptr_e - ptr, utf8_c);
            if (csz <= 0)
            {
                csz = 1;
            } else
            {
                if (b_found_del == false)
                {
                    widecharString(buf, sizeof(buf), utf8_c);
                    // OS_LOGV("%s", buf);
                    if (wchar_delimiters.find(buf) != wchar_delimiters.end())
                    {
                        b_found_del = true;
                    }
                }
                if (b_found_del)
                {
                    s = leftString.substr(0, ptr + csz - leftString.data());
                    //
                    break;
                }
            }
            ptr += csz;
        }

        return s;
    }

    // scan from ending
    std::string maxSentence()
    {
        std::string s;
        if (leftString.empty())
        {
            return s;
        }
        auto rpos = leftString.length();
        //
        auto ptr_r_b = leftString.data() + rpos - 1;
        auto ptr_r_e = leftString.data() - 1;
        //
        char buf[8];
        //
        const char *ptr_r = NULL;
        //
        size_t   csz = 0;
        uint32_t utf8_c;
        //
        bool b_found_del = false;

        ptr_r = ptr_r_b;
        //
        while (ptr_r > ptr_r_e)
        {
            utf8_c = 0;
            //
            csz = next_utf8_char(ptr_r, ptr_r - ptr_r_e, utf8_c);
            if (csz <= 0)
            {
                csz = 1;
            } else
            {
                if (b_found_del == false)
                {
                    widecharString(buf, sizeof(buf), utf8_c);
                    // OS_LOGV("%s", buf);
                    if (wchar_delimiters.find(buf) != wchar_delimiters.end())
                    {
                        b_found_del = true;
                    }
                }
                if (b_found_del)
                {
                    s = leftString.substr(0, ptr_r + csz - leftString.data());
                    //
                    break;
                }
            }
            ptr_r -= csz;
        }

        return s;
    }


    /**
     * @brief
     * - | First code point | Last code point |  Byte 1  |  Byte 2  |  Byte 3  |  Byte 4  |
     * - | :--------------: | :-------------: | :------: | :------: | :------: | :------: |
     * - |      U+0000      |     U+007F      | 0yyyzzzz |          |          |          |
     * - |      U+0080      |     U+07FF      | 110xxxyy | 10yyzzzz |          |          |
     * - |      U+0800      |     U+FFFF      | 1110wwww | 10xxxxyy | 10yyzzzz |          |
     * - |     U+010000     |    U+10FFFF     | 11110uvv | 10vvwwww | 10xxxxyy | 10yyzzzz |
     *
     * @param s
     * @param maxlen
     * @param utf8_c
     * @return size_t
     */
    static size_t next_utf8_char(const char *s, int maxlen, uint32_t &utf8_c)
    {
        if (!s || maxlen <= 0)
        {
            return 0;
        }

        size_t sz = 0;
        // If the first bit is 0, it's a single-byte character (ASCII)
        if ((*s & 0x80) == 0)
        {
            sz = 1;
        }
        // If the first three bits are 110, it's a two-byte character
        else if ((*s & 0xE0) == 0xC0)
        {
            sz = 2;
        }
        // If the first four bits are 1110, it's a three-byte character
        else if ((*s & 0xF0) == 0xE0)
        {
            sz = 3;
        }
        // If the first five bits are 11110, it's a four-byte character
        else if ((*s & 0xF8) == 0xF0)
        {
            sz = 4;
        } else
        {
            // Handle invalid UTF-8 characters
            // fprintf(stderr, "Invalid UTF-8 character encountered.\n");
            // exit(EXIT_FAILURE);
            sz = 0;
        }
        if (sz > maxlen || sz == 0)
        {
            return 0;
        }
        memcpy(&utf8_c, s, sz);
        return sz;
    }

    static size_t utf8_length(const char *s, size_t len)
    {
        size_t cnt = 0;
        if (!s || len <= 0)
        {
            return cnt;
        }
        //
        auto ptr_e = s + len;
        auto ptr_b = s;
        //
        const char *ptr = NULL;
        //
        size_t   csz = 0;
        uint32_t utf8_c;
        //

        ptr = ptr_b;
        //
        while (ptr < ptr_e)
        {
            utf8_c = 0;
            //
            csz = next_utf8_char(ptr, ptr_e - ptr, utf8_c);
            if (csz <= 0)
            {
                csz = 1;
            }
            ptr += csz;
            //
            ++cnt;
        }

        return cnt;
    }


    static size_t utf8_strncpy(char *dst_buf, size_t dst_buf_sz, const char *s, size_t len, size_t n = 0)
    {
        size_t cnt = 0;
        if (!s || len <= 0)
        {
            return cnt;
        }
        if (!dst_buf || dst_buf_sz <= 0)
        {
            return cnt;
        }
        //
        auto ptr_e = s + len;
        auto ptr_b = s;
        //
        memset(dst_buf, 0, dst_buf_sz);
        auto dst_e = dst_buf + dst_buf_sz;
        auto dst_b = dst_buf;
        //
        const char *ptr = NULL;
        char       *dst = NULL;
        //
        size_t   csz = 0;
        uint32_t utf8_c;
        //

        ptr = ptr_b;
        dst = dst_b;
        //
        while (ptr < ptr_e && dst < dst_e)
        {
            utf8_c = 0;
            //
            csz = next_utf8_char(ptr, ptr_e - ptr, utf8_c);
            if (csz <= 0)
            {
                csz = 1;
            }

            memcpy(dst, ptr, csz);

            ptr += csz;
            dst += csz;
            //
            ++cnt;
            if (n && cnt >= n)
            {
                break;
            }
        }

        return cnt;
    }
};


#endif  // __CPPHELPER_TEXTSENTENCECACHER_H__
