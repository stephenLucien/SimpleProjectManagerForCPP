
#include "cpp_helper/cpphelper_nlohmann_json.hpp"
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"



static int test_nlohmann_itr(int reason, void* userdata)
{
    auto jsItr = [](const std::string& jsStr)
    {
        //
        auto js = nlohmann_js_parse(jsStr);
        //
        for (auto& [key, value] : js.items())
        {
            OS_PRINT("js[%s]=%s", key.c_str(), value.dump().c_str());
        }
    };

    jsItr(R"(
[
        1,2,3
]
    )");

    return 0;
}

REG_TEST_FUNC(test_nlohmann_itr, test_nlohmann_itr, NULL)
