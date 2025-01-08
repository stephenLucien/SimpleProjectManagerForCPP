#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "cjson/cJSON.h"
#include "cpp_helper/cpphelper_cJSON.hpp"
#include "cpp_helper/cpphelper_os.hpp"
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

static int test_cJSON_Parse(int reason, void* userdata)
{
    auto jsonfn = "template_update.json";
    //
    if (userdata)
    {
        jsonfn = (const char*)userdata;
    }

    size_t bufsz = 32 * 1024;
    CPP_CALLOC(char, buf, bufsz);
    int read_cnt = -1;
    //
    auto str = read_data_from_file(jsonfn, buf, bufsz, &read_cnt);
    if (!str)
    {
        return -1;
    }
    CPP_CJSON_PARSE(js, str);
    //
    if (!js)
    {
        return -2;
    }
    CPP_CJSON_PRINT_UNFORMAT(info, js);
    //
    if (!info)
    {
        return -3;
    }
    OS_LOGD("%s", info);

    write_data_to_file("unformat.json", info, strlen(info));

    return 0;
}

REG_TEST_FUNC(test_cJSON_Parse, test_cJSON_Parse, NULL)
