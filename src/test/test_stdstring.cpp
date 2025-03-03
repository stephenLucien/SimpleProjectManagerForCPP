#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"


static int test_std_string(int reason, void* userdata)
{
    std::string tmp;
    //
    for (int i = 0; tmp[i] != '\0'; ++i)
    {
        OS_LOGD("%c", tmp[i]);
    }
    std::string tmp2 = "123";
    //
    for (int i = 0; tmp2[i] != '\0'; ++i)
    {
        OS_LOGD("%c", tmp2[i]);
    }
    return 0;
}

REG_TEST_FUNC(test_std_string, test_std_string, NULL)
