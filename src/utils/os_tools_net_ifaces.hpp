#ifndef OS_TOOLS_NET_IFACES_HPP
#define OS_TOOLS_NET_IFACES_HPP

#include <string>
#include <unordered_set>


class CppGetIfaces
{
   private:
    struct ifaddrs *ifaddr;

   public:
    CppGetIfaces();
    ~CppGetIfaces();

    void release();

    struct ifaddrs *get();

    std::unordered_set<std::string> getIfaces();
};

#endif /* OS_TOOLS_NET_IFACES_HPP */
