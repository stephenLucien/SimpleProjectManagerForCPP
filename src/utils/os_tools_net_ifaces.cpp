#include "os_tools_net_ifaces.hpp"

//
#include "os_tools_log.h"

//
#include <ifaddrs.h>
#include <cstring>



CppGetIfaces::CppGetIfaces()
{
    ifaddr = NULL;
    //
    if (getifaddrs(&ifaddr) == -1)
    {
        OS_LOGE("getifaddrs, %s", strerror(errno));
    }
}
CppGetIfaces::~CppGetIfaces()
{
    release();
}
void CppGetIfaces::release()
{
    if (ifaddr)
    {
        freeifaddrs(ifaddr);
        ifaddr = NULL;
    }
}
struct ifaddrs *CppGetIfaces::get()
{
    return ifaddr;
}
std::unordered_set<std::string> CppGetIfaces::getIfaces()
{
    std::unordered_set<std::string> res;
    for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }
        if (ifa->ifa_name)
        {
            res.insert(ifa->ifa_name);
        }
    }
    return res;
}
