#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
#include "utils/os_tools_net_ifaces.hpp"
#include "utils/os_tools_system.h"


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

static std::string m_iface = "dummy0";
REG_CMDLINE_ARG_PARSE_FUNC(1, 'I', "iface", 1, {
    //
    if (_param)
    {
        m_iface = _param;
    }
})

int test_dummy_iface_create(int reason, void* userdata)
{
    int ret = -1;
    //
    // $ sudo ip link add dummy1 type dummy
    // $ sudo ip addr add 2.2.2.2/24 dev dummy1
    // $ sudo ip link set dummy1 up
    SYSTEM("modprobe dummy");
    SYSTEM("ip link add %s type dummy", m_iface.c_str());
    SYSTEM("ip link set %s up", m_iface.c_str());

    return ret;
}
REG_TEST_FUNC(test_dummy_iface_create, test_dummy_iface_create, NULL)

int test_ifaces_mac(int reason, void* userdata)
{
    int ret = -1;
    //
    OS_LOGD("iface(%s): hw_ether=%s", m_iface.c_str(), OS_NET_GET_IFACE_MAC_STR(m_iface.c_str()));
    //
    ret = os_net_iface_set_hwaddr_str(m_iface.c_str(), "12:34:56:78:90:13");
    //
    OS_LOGD("iface(%s): hw_ether=%s", m_iface.c_str(), OS_NET_GET_IFACE_MAC_STR(m_iface.c_str()));
    //
    OS_LOGD("iface(%s): up=%d", m_iface.c_str(), os_net_iface_is_up(m_iface.c_str()));
    //
    os_net_set_local_ipv4_addr(m_iface.c_str(), ipv4_str2addr("192.168.113.11"));
    //
    OS_LOGD("iface(%s): ipv4 addr=%s", m_iface.c_str(), OS_NET_IFACE_GET_ADDR(m_iface.c_str()));
    OS_LOGD("iface(%s): ipv4 netmask=%s", m_iface.c_str(), OS_NET_IFACE_GET_NETMASK(m_iface.c_str()));
    OS_LOGD("iface(%s): ipv4 broadcast=%s", m_iface.c_str(), OS_NET_IFACE_GET_BRADDR(m_iface.c_str()));

    return ret;
}
REG_TEST_FUNC(test_ifaces_mac, test_ifaces_mac, NULL)
