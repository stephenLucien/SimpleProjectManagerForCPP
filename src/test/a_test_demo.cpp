#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
//
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"

//
static std::string m_test_cmd_paras = "";
//
REG_CMDLINE_ARG_PARSE_FUNC(url, 'x', "X", 1, {
    //
    if (_param)
    {
        m_test_cmd_paras = _param;
    }
})

static int test_manager_demo(int reason, void* userdata)
{
    return 0;
}

REG_TEST_FUNC(test_manager_demo, test_manager_demo, NULL)
