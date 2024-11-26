#include <cstdint>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"


static int list_itr_erase(int reason, void* userdata)
{
    std::list<std::string> l;
    //
    const uint64_t base_ts = 161;
    for (int i = 0; i < 5; i++)
    {
        l.emplace(l.end(), std::to_string(i));
    }
    {
        auto itr = l.begin();
        //
        std::advance(itr, 3);
        OS_LOGD("%s", itr->c_str());
    }
    int pos = 0;
    for (auto itr = l.begin(); itr != l.end(); pos++)
    {
        if (pos == 1)
        {
            itr = l.erase(itr);
            continue;
        }
        if (pos == 2)
        {
            itr = l.erase(itr);
            continue;
        }
        itr++;
    }
    for (auto& e : l)
    {
        OS_LOGD("%s", e.c_str());
    }
    return 0;
}

REG_TEST_FUNC(list_itr_erase, list_itr_erase, NULL)
