#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"


static int test_meminfo(int reason, void* userdata)
{
    auto ret = os_update_meminfo();
    //
    std::unordered_map<std::string, int> infos;
    //
    auto ret1 = os_get_meminfo(infos, 0);
    //
    auto avail = os_get_available_ram(0);
    //
    auto unit = os_get_meminfo_unit();
    //
    for (auto& keyval : infos)
    {
        OS_LOGI("%s: %d %s", keyval.first.c_str(), keyval.second, unit.c_str());
    }
    OS_LOGI("free: %d %s", avail, unit.c_str());

    return 0;
}

REG_TEST_FUNC(test_meminfo, test_meminfo, NULL)
