#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
#include "manager/netstatus_manager.hpp"
#include "manager/test_manager.h"



int test_netstatus_manager(void* userdata)
{
    int ret = -1;
    //
    if (NET_STATUS->run())
    {
        ret = 0;
    }

    return ret;
}


REG_TEST_FUNC(netstatus_manager, test_netstatus_manager, NULL)
