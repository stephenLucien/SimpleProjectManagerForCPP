#ifndef __CPPHELPER_IFACES_H__
#define __CPPHELPER_IFACES_H__


#include <netinet/in.h>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

//
#include "utils/os_tools_net.h"


class CppGetIfaces
{
   private:
    struct ifaddrs *ifaddr;

   public:
    CppGetIfaces();
    ~CppGetIfaces();
    void            release();
    struct ifaddrs *get();

    class AddressV4
    {
       public:
        AddressV4()
        {
            INVALID_IP4(&addr);
            INVALID_IP4(&mask);
            INVALID_IP4(&prefix);
        }
        ~AddressV4()
        {
        }
        //
        struct in_addr addr;
        struct in_addr mask;
        struct in_addr prefix;

        void dump();
    };

    class AddressV6
    {
       public:
        AddressV6()
        {
            INVALID_IP6(&addr);
            INVALID_IP6(&mask);
            INVALID_IP6(&prefix);
        }
        ~AddressV6()
        {
        }
        //
        struct in6_addr addr;
        struct in6_addr mask;
        struct in6_addr prefix;

        void dump();
    };

    class IfaceInfo
    {
       public:
        IfaceInfo()
        {
            flags = 0;
        }
        ~IfaceInfo()
        {
        }
        std::string iface;
        //
        unsigned int flags;
        //
        std::list<AddressV4> ipv4_addrs;
        //
        std::list<AddressV6> ipv6_addrs;
        //
        void dump();
    };

    std::unordered_map<std::string, IfaceInfo> getInfos(bool dump = false);

    static std::unordered_set<std::string> getIfaces(std::unordered_map<std::string, IfaceInfo> &infos, bool dump = false)
    {
        std::unordered_set<std::string> res;
        //
        for (auto &e : infos)
        {
            res.insert(e.first);
            if (dump)
            {
                e.second.dump();
            }
        }
        return res;
    }

    static void iface_ipv4_ipv6_support(std::unordered_map<std::string, IfaceInfo> &infos,
                                        const std::string                          &iface,
                                        bool                                       &ipv4_supported,
                                        bool                                       &ipv6_supported)
    {
        ipv4_supported = false;
        ipv6_supported = false;
        //
        for (auto &e : infos)
        {
            if (e.first != iface)
            {
                continue;
            }
            if (!e.second.ipv4_addrs.empty())
            {
                ipv4_supported = true;
            }
            if (!e.second.ipv6_addrs.empty())
            {
                ipv6_supported = true;
            }
            break;
        }
    }
};

int os_tools_iface_get_sys_info(std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &ifaces_infos);

#endif  // __CPPHELPER_IFACES_H__
