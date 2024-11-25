#include "cpp_helper/cpphelper_nlohmann_json.hpp"
#include "utils/os_tools.h"

//
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

static const char* test_file = "test.json";
//
static const char* test_data = R"({
   "brightness" : 20,
   "distance" : 0,
   "ismute" : 0,
   "mic" : 1,
   "nightMode" : 0,
   "volume" : 20
})";

int test_text_file_read(int reason, void* userdata)
{
    int ret = -1;
    //
    auto fn = (const char*)userdata;
    if (!fn)
    {
        return ret;
    }
    //
    auto str = read_text_file(fn);
    //
    OS_LOGI("read from %s:\n%s", fn, str.c_str());
    ret = str.length();
    //
    auto js = nlohmann_js_parse(str);
    //
    int brightness = -1;
    nlohmann_jsobj_try_get_value_debug(js, "brightness", brightness);
    OS_LOGI("brightness:%d", brightness);

    std::string var1;
    nlohmann_jsobj_try_get_value_debug(js, "brightness", var1);
    OS_LOGI("var1:%s", var1.c_str());

    return ret;
}
REG_TEST_FUNC(text_file_read, test_text_file_read, test_file)


int test_text_file_write(int reason, void* userdata)
{
    int ret = -1;
    //
    auto str = test_data;
    //
    auto fn = (const char*)userdata;
    if (!fn)
    {
        return ret;
    }
    //
    ret = write_text_file(fn, str);

    return ret;
}
REG_TEST_FUNC(text_file_write, test_text_file_write, test_file)
