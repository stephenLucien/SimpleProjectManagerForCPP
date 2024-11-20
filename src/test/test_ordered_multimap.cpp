#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"

static void my_omm_insert(std::multimap<int, int>& umm, int key, int value)
{
    auto tmpSet = umm.equal_range(key);
    for (auto tmpItr = tmpSet.first; tmpItr != tmpSet.second; tmpItr++)
    {
        if (tmpItr->second > value)
        {
            OS_LOGI("key:%d, insert %d into %d", key, value, tmpItr->second);
            umm.emplace_hint(tmpItr, key, value);
            return;
        }
    }

    OS_LOGI("key:%d, insert %d", key, value);
    umm.emplace(key, value);
}

int test_ordered_multimap(void* userdata)
{
    std::multimap<int, int> umm;  // Initial elements

    my_omm_insert(umm, 1, 3);
    my_omm_insert(umm, 1, 5);
    my_omm_insert(umm, 1, 2);
    umm.emplace(2, 3);
    umm.emplace(2, 5);
    umm.emplace(2, 2);

    for (const auto& e : umm)
    {
        OS_LOGI("<%d,%d> count=%zu", e.first, e.second, umm.count(e.first));
    }
    return 0;
}


REG_TEST_FUNC(ordered_multimap, test_ordered_multimap, NULL)
