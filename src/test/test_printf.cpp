#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"


static int test_printf(int reason, void* userdata)
{
    for (uint32_t i = 0; i < 64; ++i)
    {
        OS_LOGD("%.2u %02u", i, i);
    }
    return 0;
}

REG_TEST_FUNC(test_printf, test_printf, NULL)
