#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"


static int test_map_itr_erase(int reason, void* userdata)
{
    std::map<uint64_t, int> m;
    //
    const uint64_t base_ts = 161;
    for (int i = 0; i < 5; i++)
    {
        m[base_ts * i] = base_ts - i;
    }

    for (auto& e : m)
    {
        OS_LOGD("%llu:%d", e.first, e.second);
    }
    for (auto itr = m.begin(); itr != m.end();)
    {
        if (itr->second == base_ts - 3)
        {
            itr = m.erase(itr);
            continue;
        }
        if (itr->second == base_ts - 2)
        {
            itr = m.erase(itr);
            continue;
        }
        OS_LOGD("%llu:%d", itr->first, itr->second);
        itr++;
    }
    for (auto& e : m)
    {
        OS_LOGD("%llu:%d", e.first, e.second);
    }
    return 0;
}

REG_TEST_FUNC(map_itr_erase, test_map_itr_erase, NULL)
