#include "utils/os_tools.h"
#include "utils/os_tools_net.h"

//
#include "manager/netstatus_manager.hpp"
#include "manager/test_manager.h"

int test_nslookup(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* hostname = "www.baidu.com";
    //
    if (userdata)
    {
        hostname = (const char*)userdata;
    }

    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    ret = os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs);
    if (ret)
    {
        OS_LOGD("nslookup fail");
        return ret;
    }

    int ip_cnt = 0;
    OS_LOGD("hostname: %s", hostname);
    //
    for (auto& ip4 : ipv4_addrs)
    {
        ip_cnt++;
        OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    }
    //
    for (auto& ip6 : ipv6_addrs)
    {
        ip_cnt++;
        OS_LOGD("ip6: %s", IPV4_ADDR2STR(&ip6));
    }

    return ip_cnt;
}


int test_icmp_ping4(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* hostname = "www.baidu.com";
    //
    if (userdata)
    {
        hostname = (const char*)userdata;
    }

    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    ret = os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs);
    if (ret)
    {
        OS_LOGD("nslookup fail");
        return ret;
    }

    OS_LOGD("hostname: %s", hostname);
    //
    for (auto& ip4 : ipv4_addrs)
    {
        // OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    }
    //
    for (auto& ip6 : ipv6_addrs)
    {
        // OS_LOGD("ip6: %s", IPV4_ADDR2STR(&ip6));
    }

    if (ipv4_addrs.empty())
    {
        OS_LOGD("ipv4 empty");
        return -1;
    }

    auto ip4 = ipv4_addrs.front();


    OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    ret = os_net_icmp_ping4(ip4, 1, 1, NULL);


    return ret;
}


int test_tcp_ping4(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* hostname = "www.baidu.com";
    //
    if (userdata)
    {
        hostname = (const char*)userdata;
    }

    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    ret = os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs);
    if (ret)
    {
        OS_LOGD("nslookup fail");
        return ret;
    }

    OS_LOGD("hostname: %s", hostname);
    //
    for (auto& ip4 : ipv4_addrs)
    {
        // OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    }
    //
    for (auto& ip6 : ipv6_addrs)
    {
        // OS_LOGD("ip6: %s", IPV4_ADDR2STR(&ip6));
    }

    if (ipv4_addrs.empty())
    {
        OS_LOGD("ipv4 empty");
        return -1;
    }

    auto ip4 = ipv4_addrs.front();


    OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    ret = os_net_tcp_ping4(ip4, 80, 1, 1, NULL);


    return ret;
}


int test_icmp_ping6(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* hostname = "www.baidu.com";
    //
    if (userdata)
    {
        hostname = (const char*)userdata;
    }

    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    ret = os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs);
    if (ret)
    {
        OS_LOGD("nslookup fail");
        return ret;
    }

    OS_LOGD("hostname: %s", hostname);
    //
    for (auto& ip4 : ipv4_addrs)
    {
        // OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    }
    //
    for (auto& ip6 : ipv6_addrs)
    {
        // OS_LOGD("ip6: %s", IPV4_ADDR2STR(&ip6));
    }

    if (ipv6_addrs.empty())
    {
        OS_LOGD("ipv6 empty");
        return -1;
    }

    auto ip6 = ipv6_addrs.front();


    OS_LOGD("ip6: %s", IPV6_ADDR2STR(&ip6));
    ret = os_net_icmp_ping6(ip6, 1, 1, NULL);


    return ret;
}

int test_tcp_ping6(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* hostname = "www.baidu.com";
    //
    if (userdata)
    {
        hostname = (const char*)userdata;
    }

    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    ret = os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs);
    if (ret)
    {
        OS_LOGD("nslookup fail");
        return ret;
    }

    OS_LOGD("hostname: %s", hostname);
    //
    for (auto& ip4 : ipv4_addrs)
    {
        // OS_LOGD("ip4: %s", IPV4_ADDR2STR(&ip4));
    }
    //
    for (auto& ip6 : ipv6_addrs)
    {
        // OS_LOGD("ip6: %s", IPV4_ADDR2STR(&ip6));
    }

    if (ipv6_addrs.empty())
    {
        OS_LOGD("ipv6 empty");
        return -1;
    }

    auto ip6 = ipv6_addrs.front();


    OS_LOGD("ip6: %s", IPV6_ADDR2STR(&ip6));
    ret = os_net_tcp_ping6(ip6, 80, 1, 1, NULL);


    return ret;
}



int test_icmp_wan6(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* testIP6 = "2606:4700:4700::1111";
    //
    auto ip6 = ipv6_str2addr(testIP6);

    OS_LOGD("ip6: %s", IPV6_ADDR2STR(&ip6));
    ret = os_net_icmp_ping6(ip6, 1, 1, NULL);


    return ret;
}

int test_tcp_wan6(int reason, void* userdata)
{
    int ret = -1;
    //
    const char* testIP6 = "2606:4700:4700::1111";
    //
    auto ip6 = ipv6_str2addr(testIP6);

    OS_LOGD("ip6: %s", IPV6_ADDR2STR(&ip6));
    ret = os_net_tcp_ping6(ip6, 80, 1, 1, NULL);


    return ret;
}

int test_netstatus_manager(int reason, void* userdata)
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

REG_TEST_FUNC(nslookup, test_nslookup, NULL)
REG_TEST_FUNC(nslookup_baidu, test_nslookup, "www.baidu.com")
REG_TEST_FUNC(nslookup_google, test_nslookup, "www.google.com")

REG_TEST_FUNC(icmp_ping4, test_icmp_ping4, NULL)
REG_TEST_FUNC(tcp_ping4, test_tcp_ping4, NULL)

REG_TEST_FUNC(icmp_ping6, test_icmp_ping6, NULL)
REG_TEST_FUNC(tcp_ping6, test_tcp_ping6, NULL)
REG_TEST_FUNC(icmp_wan6, test_icmp_wan6, NULL)
REG_TEST_FUNC(tcp_wan6, test_tcp_wan6, NULL)
