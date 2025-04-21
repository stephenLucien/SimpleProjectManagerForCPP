#include "arg_parser.h"

//
#include <getopt.h>
#include <cstddef>
#include <vector>
#include "os_tools_log.h"


int arg_parser(const ArgumentData* arg_datas, int arg_datas_cnt, int argc, char* argv[])
{
    int ret = -1;
    //
    if (!arg_datas || arg_datas_cnt <= 0)
    {
        return ret;
    }
    ret = 0;
    //
    std::vector<char> tmpShortOptions(2 * arg_datas_cnt + 1);
    {
        //
        int i      = 0;
        int offset = 0;
        for (; i < arg_datas_cnt; ++i)
        {
            tmpShortOptions[offset++] = arg_datas[i].val;
            if (arg_datas[i].has_arg)
            {
                tmpShortOptions[offset++] = ':';
            }
        }
        tmpShortOptions[offset++] = '\0';
    }
    auto short_option = tmpShortOptions.data();
    //
    std::vector<struct option> tmpLongOptions(arg_datas_cnt + 1);
    {
        //
        int i = 0;
        for (; i < arg_datas_cnt; ++i)
        {
            tmpLongOptions[i] = {.name = arg_datas[i].name, .has_arg = arg_datas[i].has_arg, .flag = NULL, .val = arg_datas[i].val};
        }
        tmpLongOptions[i] = {.name = NULL, .has_arg = 0, .flag = NULL, .val = 0};
    }
    auto long_option = tmpLongOptions.data();

    do
    {
        auto c = getopt_long(argc, argv, short_option, long_option, NULL);
        if (c < 0)
        {
            break;
        }
        for (int i = 0; i < arg_datas_cnt; ++i)
        {
            if (arg_datas[i].val == c)
            {
                OS_LOGI("-%c --%s %s", arg_datas[i].val, arg_datas[i].name, arg_datas[i].has_arg ? optarg : "");
                ++ret;
                if (arg_datas[i].atArgExist)
                {
                    arg_datas[i].atArgExist(arg_datas[i].val, arg_datas[i].name, arg_datas[i].has_arg ? optarg : NULL);
                }
                break;
            }
        }

    } while (1);

    return ret;
}
