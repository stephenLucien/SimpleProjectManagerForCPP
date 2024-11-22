#include <iostream>
#include <string>
#include <unordered_map>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"

static void my_umm_insert(std::unordered_multimap<int, int>& umm, int key, int value)
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

int test_unordered_multimap(int reason, void* userdata)
{
    std::unordered_multimap<int, int> umm;  // Initial elements

    my_umm_insert(umm, 1, 3);
    my_umm_insert(umm, 1, 5);
    my_umm_insert(umm, 1, 2);
    umm.emplace(2, 3);
    umm.emplace(2, 5);
    umm.emplace(2, 2);

    for (int b = 0; b < (int)umm.bucket_count(); b++)
    {
        auto bsz = umm.bucket_size(b);
        if (bsz > 0)
        {
            if (umm.begin(b) != umm.end(b))
            {
                OS_LOGI("<%d,%d>", umm.begin(b)->first, umm.begin(b)->second);
            }
        }
    }

    for (const auto& e : umm)
    {
        OS_LOGI("<%d,%d> count=%zu", e.first, e.second, umm.count(e.first));
    }
    return 0;
}


REG_TEST_FUNC(unordered_multimap, test_unordered_multimap, NULL)
