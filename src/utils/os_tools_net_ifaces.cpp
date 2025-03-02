#include "os_tools_net_ifaces.hpp"

//
#include "common.h"
#include "cpp_helper/cpphelper_os.hpp"
#include "os_tools_log.h"
#include "os_tools_net.h"
#include "os_tools_system.h"

//
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/dir.h>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>


void CppGetIfaces::AddressV4::dump()
{
    OS_LOGD("[ipv4] addr=%s mask=%s prefix=%s\n", IPV4_ADDR2STR(&addr), IPV4_ADDR2STR(&mask), IPV4_ADDR2STR(&prefix));
}


void CppGetIfaces::AddressV6::dump()
{
    OS_LOGD("[ipv6] addr=%s mask=%s prefix=%s\n", IPV6_ADDR2STR(&addr), IPV6_ADDR2STR(&mask), IPV6_ADDR2STR(&prefix));
}

void CppGetIfaces::IfaceInfo::dump()
{
    OS_LOGD("ifa_name:%s", iface.c_str());
    OS_LOGD("ifa_flags:0x%08X", flags);
    for (auto &e : ipv4_addrs)
    {
        e.dump();
    }
    for (auto &e : ipv6_addrs)
    {
        e.dump();
    }
}


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


std::unordered_map<std::string, CppGetIfaces::IfaceInfo> CppGetIfaces::getInfos(bool dump)
{
    std::unordered_map<std::string, IfaceInfo> infos;

    for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_name)
        {
            continue;
        }
        if (dump)
        {
            OS_LOGD("ifa_name:%s ifa_flags:0x%x inet:%x\n", ifa->ifa_name, ifa->ifa_flags, ifa->ifa_addr->sa_family);
        }
        auto node = infos.find(ifa->ifa_name);
        if (node == infos.end())
        {
            infos[ifa->ifa_name] = IfaceInfo();
        }
        //
        node = infos.find(ifa->ifa_name);
        if (node == infos.end())
        {
            continue;
        }
        node->second.iface = ifa->ifa_name;
        node->second.flags = ifa->ifa_flags;

        if (ifa->ifa_addr == NULL || ifa->ifa_netmask == NULL)
        {
            continue;
        }
        //
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            auto addr = (struct sockaddr_in *)(ifa->ifa_addr);
            auto mask = (struct sockaddr_in *)(ifa->ifa_netmask);
            //
            struct in_addr prefix;
            //
            prefix.s_addr = addr->sin_addr.s_addr & mask->sin_addr.s_addr;

            AddressV4 ip;
            //
            ip.addr   = addr->sin_addr;
            ip.mask   = mask->sin_addr;
            ip.prefix = prefix;
            //
            node->second.ipv4_addrs.push_back(ip);
            //
            if (dump)
            {
                OS_LOGD("addr:%s mask:%s prefix:%s\n", IPV4_ADDR2STR(&ip.addr), IPV4_ADDR2STR(&ip.mask), IPV4_ADDR2STR(&ip.prefix));
            }
        } else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            auto addr = (struct sockaddr_in6 *)(ifa->ifa_addr);
            auto mask = (struct sockaddr_in6 *)(ifa->ifa_netmask);
            //
            struct in6_addr prefix;

            for (int i = 0; i < ARRSZ(prefix.s6_addr); i++)
            {
                prefix.s6_addr[i] = addr->sin6_addr.s6_addr[i] & mask->sin6_addr.s6_addr[i];
            }

            AddressV6 ip;
            //
            ip.addr   = addr->sin6_addr;
            ip.mask   = mask->sin6_addr;
            ip.prefix = prefix;
            //
            node->second.ipv6_addrs.push_back(ip);
            //
            if (dump)
            {
                OS_LOGD("addr:%s mask:%s prefix:%s\n", IPV6_ADDR2STR(&ip.addr), IPV6_ADDR2STR(&ip.mask), IPV6_ADDR2STR(&ip.prefix));
            }
        }
    }

    return infos;
}



int os_tools_iface_get_sys_info(std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &ifaces_infos)
{
    int ret = -1;
    //
    const char *sys_path = "/sys/class/net";
    //
    ifaces_infos.clear();
    {
        std::list<std::string> entries;
        //
        ret = os_get_dir_entries(sys_path, entries, DT_DIR);
        if (ret < 0)
        {
            OS_LOGE("");
            return ret;
        }
        //
        for (auto &e : entries)
        {
            ifaces_infos[e] = std::unordered_map<std::string, std::string>();
        }
    }
    if (ifaces_infos.empty())
    {
        OS_LOGV("");
        return 0;
    }

    for (auto itr = ifaces_infos.begin(); itr != ifaces_infos.end();)
    {
        std::string path = std::string(sys_path) + std::string("/") + itr->first;
        //
        std::list<std::string> entries;
        //
        ret = os_get_dir_entries(path, entries, DT_REG);
        if (ret < 0)
        {
            itr = ifaces_infos.erase(itr);
            OS_LOGV("");
            continue;
        } else
        {
            for (auto &fn : entries)
            {
                std::string f = path + std::string("/") + fn;
                //
                auto txt = read_text_file(f);
                //
                if (txt.length() > 0)
                {
                    itr->second.insert(std::pair<std::string, std::string>(fn, txt));
                }
            }
            //
            ++itr;
        }
    }

    return 0;
}
