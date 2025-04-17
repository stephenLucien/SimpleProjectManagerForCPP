#include <inttypes.h>
#include <unistd.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
//
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"

#include "json2cppclass/JsonClassAPI_test.hpp"
#include "utils/os_tools_system.h"



static int test_json_class(int reason, void* userdata)
{
    int ret = -1;
    //
    auto jsStr = read_text_file("testCppClass.json");
    //
    auto js = nlohmannUser::JsonClassAPI::parseJsonString(jsStr);
    //
    nlohmannUserArrayContainer(TestJsonClassAPI) array;
    //
    nlohmannUser::JsonClassAPI::_JsonGetArrayClass(js, "data", array);
    //
    for (const auto& e : array)
    {
        OS_LOGI("\n%s\n", e.toJsonString(4).c_str());
    }

    return 0;
}

REG_TEST_FUNC(test_json_class, test_json_class, NULL)
