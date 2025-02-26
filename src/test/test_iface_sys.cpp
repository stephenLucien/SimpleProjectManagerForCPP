#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
#include "manager/test_manager.h"
#include "utils/os_tools_net_ifaces.hpp"


int test_ifaces_sys(int reason, void* userdata)
{
    int ret = -1;
    //
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> ifaces_infos;
    //
    ret = os_tools_iface_get_sys_info(ifaces_infos);
    //
    for (auto& ifn : ifaces_infos)
    {
        for (auto& pn : ifn.second)
        {
            OS_LOGD("iface(%s):%s=%s\n", ifn.first.c_str(), pn.first.c_str(), pn.second.c_str());
        }
    }

    return ret;
}


REG_TEST_FUNC(test_ifaces_sys, test_ifaces_sys, NULL)
