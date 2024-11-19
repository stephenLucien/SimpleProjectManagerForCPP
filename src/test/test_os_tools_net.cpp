#include "test.h"

//
#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
int test_os_tools_net(void* userdata)
{
    int ret = -1;
    //
    auto hostname = "one.one.one.one";

    //
    auto ip4 = os_net_nslookup4(hostname);
    if (IS_INVALID_IP4(&ip4))
    {
        if (is_sudo(0))
        {
            //
            auto r1 = os_net_icmp_ping4(ip4, 2, 2, NULL);
            OS_LOGI("icmp4: %d", r1);
        }
        //
        auto r2 = os_net_tcp_ping4(ip4, 80, 2, 2, NULL);
        OS_LOGI("tcp4: %d", r2);
    }

    //
    auto ip6 = os_net_nslookup6(hostname);
    if (IS_INVALID_IP6(&ip6))
    {
        if (is_sudo(0))
        {
            //
            auto r1 = os_net_icmp_ping6(ip6, 2, 2, NULL);
            OS_LOGI("icmp6: %d", r1);
        }
        //
        auto r2 = os_net_tcp_ping6(ip6, 80, 2, 2, NULL);
        OS_LOGI("tcp6: %d", r2);
    }

    ret = 0;

    return ret;
}


REG_TEST_FUNC(os_tools_net, test_os_tools_net, NULL)
