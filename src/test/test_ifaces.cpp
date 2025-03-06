#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
#include "utils/os_tools_net_ifaces.hpp"
#include "utils/os_tools_net_socket.hpp"
#include "utils/os_tools_system.h"


int test_ifaces_dump(int reason, void *userdata)
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

int test_dummy_iface_create(int reason, void *userdata)
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

int test_ifaces_mac(int reason, void *userdata)
{
    int ret = -1;
    //
    OS_LOGD("iface(%s): hw_ether=%s", m_iface.c_str(), OS_NET_IFACE_GET_MAC_STR(m_iface.c_str()));
    //
    ret = os_net_iface_set_hwaddr_str(m_iface.c_str(), "12:34:56:78:90:13");
    //
    OS_LOGD("iface(%s): hw_ether=%s", m_iface.c_str(), OS_NET_IFACE_GET_MAC_STR(m_iface.c_str()));
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



static int iface_lan_ok(const char *iface, int timeout_sec = 1, int b_print_log = 0)
{
    std::string gw = OS_NET_IFACE_GET_GWADDR(iface);
    //
    OS_LOGI("gw:%s", gw.c_str());
    //
    if (gw.empty())
    {
        return 0;
    }
    auto addr = gw.c_str();
    //
    auto ip4 = ipv4_str2addr(addr);

    // ICMP ping
    if (os_net_icmp_ping4(ip4, timeout_sec, timeout_sec, iface) == 0)
    {
        return 1;
    }
    // web
    if (os_net_tcp_ping4(ip4, 80, timeout_sec, timeout_sec, iface) == 0 || os_net_tcp_ping4(ip4, 443, timeout_sec, timeout_sec, iface) == 0
        || os_net_tcp_ping4(ip4, 8080, timeout_sec, timeout_sec, iface) == 0)
    {
        return 2;
    }

    // dns port
    if (os_net_tcp_ping4(ip4, 53, timeout_sec, timeout_sec, iface) == 0 || os_net_tcp_ping4(ip4, 5353, timeout_sec, timeout_sec, iface) == 0)
    {
        return 4;
    }

    // ntp port
    if (os_net_tcp_ping4(ip4, 123, timeout_sec, timeout_sec, iface) == 0)
    {
        return 8;
    }

    // remote access port: ssh telnet vnc
    if (os_net_tcp_ping4(ip4, 22, timeout_sec, timeout_sec, iface) == 0 || os_net_tcp_ping4(ip4, 23, timeout_sec, timeout_sec, iface) == 0
        || os_net_tcp_ping4(ip4, 5900, timeout_sec, timeout_sec, iface) == 0)
    {
        return 16;
    }

    // nfs: ftp_data ftp_file tftp nfs smb
    if (os_net_tcp_ping4(ip4, 20, timeout_sec, timeout_sec, iface) == 0 || os_net_tcp_ping4(ip4, 21, timeout_sec, timeout_sec, iface) == 0
        || os_net_tcp_ping4(ip4, 69, timeout_sec, timeout_sec, iface) == 0 || os_net_tcp_ping4(ip4, 2049, timeout_sec, timeout_sec, iface) == 0
        || os_net_tcp_ping4(ip4, 445, timeout_sec, timeout_sec, iface) == 0)
    {
        return 32;
    }

    return 0;
}

int get_gatewayip_test(int reason, void *userdata)
{
    int ret = -1;
    //
    ret = iface_lan_ok(NULL);
    //

    return ret;
}
REG_TEST_FUNC(get_gatewayip_test, get_gatewayip_test, NULL)
