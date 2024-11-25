#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
#include "manager/test_manager.h"
#include "utils/os_tools_net_ifaces.hpp"


int test_ifaces_dump(int reason, void* userdata)
{
    int ret = -1;
    //
    CppGetIfaces ifaces;
    //
    auto infos = ifaces.getInfos(true);
    //
    auto eles = ifaces.getIfaces(infos, true);

    ret = 0;

    return ret;
}


REG_TEST_FUNC(ifaces_dump, test_ifaces_dump, NULL)
