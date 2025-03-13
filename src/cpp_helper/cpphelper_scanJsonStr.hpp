#ifndef __CPPHELPER_SCANJSONSTR_H__
#define __CPPHELPER_SCANJSONSTR_H__


#include <cstddef>
#include <cstdint>


class ScanJsonStringInplace
{
   private:
    //
    char *buf = NULL;
    //
    size_t offset = 0;
    //
    char *js_begin = 0;
    //
    char *js_end = 0;
    // '{'
    uint16_t scope_cnt_begin = 0;
    // '}'
    uint16_t scope_cnt_end = 0;

   public:
    void init(void *buf)
    {
        this->buf             = (char *)buf;
        this->offset          = 0;
        this->js_begin        = NULL;
        this->js_end          = NULL;
        this->scope_cnt_begin = 0;
        this->scope_cnt_end   = 0;
    }
    size_t scan(size_t step = 1, char **js_b = NULL, char **js_e = NULL)
    {
        size_t cnt = 0;
        //
        bool found = false;
        //
        auto ptr_b = buf + offset;
        auto ptr_e = ptr_b + step;
        //
        auto ptr = ptr_b;
        while (ptr != ptr_e)
        {
            int b_check = 0;
            if (ptr[0] == '{')
            {
                ++scope_cnt_begin;
                if (!js_begin)
                {
                    js_begin = ptr;
                }
            } else if (ptr[0] == '}')
            {
                ++scope_cnt_end;
                b_check = 1;
            }
            if (b_check && scope_cnt_begin == scope_cnt_end)
            {
                //
                js_end = ptr;
                //
                found = true;
            }
            ++ptr;
            ++cnt;
            if (found)
            {
                break;
            }
        }
        offset += cnt;

        if (found)
        {
            if (js_b)
            {
                *js_b = js_begin;
            }
            if (js_e)
            {
                *js_e = js_end;
            }
            js_begin = NULL;
            js_end   = NULL;
        }

        return cnt;
    }
};

#endif  // __CPPHELPER_SCANJSONSTR_H__
