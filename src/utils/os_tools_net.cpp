#include "os_tools_net.h"


//
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>
#include <list>
#include <vector>


//
#include "os_tools.h"


int os_net_get_local_ipv4_addr(const char *iface, struct in_addr &ip4)
{
    int sa_family = AF_INET;
    //
    CppSocket sock(sa_family, SOCK_DGRAM, 0);
    if (!sock.isValid())
    {
        return -1;
    }
    auto fd = sock.get();
    //
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = sa_family;
    if (iface)
    {
        strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    }

    if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
    {
        return -1;
    }

    ip4 = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    return 0;
}


int os_net_get_local_ipv6_addr(const char *iface, struct in6_addr &ip6)
{
    int sa_family = AF_INET6;
    //
    CppSocket sock(sa_family, SOCK_DGRAM, 0);
    if (!sock.isValid())
    {
        return -1;
    }
    //
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = sa_family;
    if (iface)
    {
        strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    }

    if (ioctl(sock.get(), SIOCGIFADDR, &ifr) != 0)
    {
        return -1;
    }

    ip6 = ((struct sockaddr_in6 *)&ifr.ifr_addr)->sin6_addr;

    return 0;
}


int socket_bind_to_ipv4_addr(int fd, struct in_addr ipv4_addr, uint16_t port)
{
    return CppSocket::bind_to_ipv4_addr(fd, ipv4_addr, port);
}

int socket_bind_to_ipv6_addr(int fd, struct in6_addr ipv6_addr, uint16_t port)
{
    return CppSocket::bind_to_ipv6_addr(fd, ipv6_addr, port);
}

int socket_bind_to_device(int fd, const char *iface_name)
{
    return CppSocket::bind_to_device(fd, iface_name);
}

int os_net_nslookup(const char *hostname, std::list<struct in_addr> &ipv4_addrs, std::list<struct in6_addr> &ipv6_addrs)
{
    ipv4_addrs.clear();
    ipv6_addrs.clear();
    //
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family    = PF_UNSPEC;
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags    |= AI_CANONNAME;
    //
    auto errcode = getaddrinfo(hostname, NULL, &hints, &result);
    //
    if (errcode != 0)
    {
        // OS_LOGE("getaddrinfo");
        return -1;
    }
    //
    auto itr = result;
    //
    while (itr)
    {
        switch (itr->ai_family)
        {
            case AF_INET:
            {
                auto addr = ((struct sockaddr_in *)itr->ai_addr)->sin_addr;
                ipv4_addrs.push_back(addr);
                // OS_LOGV("address: %s (%s)\n", IPV4_ADDR2STR(&addr), itr->ai_canonname ? itr->ai_canonname : "null");
            }
            break;
            case AF_INET6:
            {
                auto addr = ((struct sockaddr_in6 *)itr->ai_addr)->sin6_addr;
                ipv6_addrs.push_back(addr);
                // OS_LOGV("address: %s (%s)\n", IPV6_ADDR2STR(&addr), itr->ai_canonname ? itr->ai_canonname : "null");
            }
            break;
            default:
            {
            }
            break;
        }
        itr = itr->ai_next;
    }
    freeaddrinfo(result);
    return errcode;
}

struct in_addr os_net_nslookup4(const char *hostname)
{
    struct in_addr sin_addr;
    INVALID_IP4(&sin_addr);
    //
    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    //
    if (os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs) != 0)
    {
        return sin_addr;
    }
    auto itr = ipv4_addrs.begin();
    if (itr != ipv4_addrs.end())
    {
        sin_addr = *itr;
    }
    return sin_addr;
}

struct in6_addr os_net_nslookup6(const char *hostname)
{
    struct in6_addr sin_addr;
    INVALID_IP6(&sin_addr);
    //
    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    //
    if (os_net_nslookup(hostname, ipv4_addrs, ipv6_addrs) != 0)
    {
        return sin_addr;
    }
    auto itr = ipv6_addrs.begin();
    if (itr != ipv6_addrs.end())
    {
        sin_addr = *itr;
    }
    return sin_addr;
}

int os_net_tcp_ping4(struct in_addr ipv4_addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
    int ret = -1;
    //
    CppSocket sock(PF_INET, SOCK_STREAM, IPPROTO_IP);
    if (!sock.isValid())
    {
        return ret;
    }
    //
    sock.set_reuse_addr(1);
    sock.set_reuse_port(1);
    sock.set_send_timeout(send_timeout_sec);
    sock.set_recv_timeout(recv_timeout_sec);
    sock.set_tcp_nodelay(1);
    if (iface && sock.bind_to_device(iface) == -1)
    {
        return ret;
    }
    ret = sock.connect_ipv4(ipv4_addr, port);
    //
    return ret;
}

int os_net_tcp_ping6(struct in6_addr ipv6_addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
    int ret = -1;
    //
    CppSocket sock(PF_INET, SOCK_STREAM, IPPROTO_IP);
    if (!sock.isValid())
    {
        return ret;
    }
    //
    sock.set_reuse_addr(1);
    sock.set_reuse_port(1);
    sock.set_send_timeout(send_timeout_sec);
    sock.set_recv_timeout(recv_timeout_sec);
    sock.set_tcp_nodelay(1);
    if (iface && sock.bind_to_device(iface) == -1)
    {
        return ret;
    }
    ret = sock.connect_ipv6(ipv6_addr, port);
    //
    return ret;
}


static inline uint16_t icmp_checksum(uint16_t *buf, int bufsz)
{
    unsigned long sum = 0xffff;
    while (bufsz > 1)
    {
        sum += *buf;
        buf++;
        bufsz -= 2;
    }
    if (bufsz == 1)
    {
        sum += *(unsigned char *)buf;
    }
    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}


int os_net_icmp_ping4(struct in_addr ip4, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
    int ret = -1;
    //
    int dump = 0;
    //
    CppSocket sock(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (!sock.isValid())
    {
        OS_LOGE("icmp socket, %s", strerror(errno));
        return ret;
    }
    //
    sock.set_reuse_addr(1);
    sock.set_reuse_port(1);
    sock.set_send_timeout(send_timeout_sec);
    sock.set_recv_timeout(recv_timeout_sec);
    if (iface && sock.bind_to_device(iface) == -1)
    {
        return ret;
    }
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = PF_INET;  // IPv4
    addr.sin_addr   = ip4;

    //
    struct icmphdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.type             = ICMP_ECHO;
    hdr.code             = 0;
    hdr.checksum         = 0;
    hdr.un.echo.id       = 0;
    hdr.un.echo.sequence = 0;
    hdr.checksum         = icmp_checksum((uint16_t *)&hdr, sizeof(hdr));
    //
    ret = sendto(sock.get(), (char *)&hdr, sizeof(hdr), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        return ret;
    }
    uint8_t buf[1024];
    ret = sock.read_data(buf, sizeof(buf));
    if (ret == -1)
    {
        return ret;
    }
    //
    auto iphdrptr = (struct iphdr *)buf;
    //
    auto icmphdrptr = (struct icmphdr *)(buf + (iphdrptr->ihl) * 4);

    ret = -1;
    //
    switch (icmphdrptr->type)
    {
        case 3:
        {
            if (dump)
            {
                OS_LOGI(
                    "(%s)The host %s is unreachable, "
                    "ICMP_type=%d, ICMP_code=%d ",
                    iface ? iface : "all",
                    IPV4_ADDR2STR(&ip4),
                    icmphdrptr->type,
                    icmphdrptr->code);
            }
        }
        break;
        case 8:
        {
            if (dump)
            {
                OS_LOGI("(%s)The host %s is alive, ICMP_type=%d, ICMP_code=%d ",

                        iface ? iface : "all",
                        IPV4_ADDR2STR(&ip4),
                        icmphdrptr->type,
                        icmphdrptr->code);
            }
            ret = 0;
        }
        break;
        case 0:
        {
            if (dump)
            {
                OS_LOGI("(%s)The host %s is alive, ICMP_type=%d, ICMP_code=%d ",

                        iface ? iface : "all",
                        IPV4_ADDR2STR(&ip4),
                        icmphdrptr->type,
                        icmphdrptr->code);
            }
            ret = 0;
        }

        break;
        default:
        {
            if (dump)
            {
                OS_LOGI(
                    "(%s)The host %s, unknown situation, "
                    "ICMP_type=%d, ICMP_code=%d ",
                    iface ? iface : "all",
                    IPV4_ADDR2STR(&ip4),
                    icmphdrptr->type,
                    icmphdrptr->code);
            }
        }
        break;
    }

    return ret;
}


int os_net_icmp_ping6(struct in6_addr ip6, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
    int ret = -1;
    //
    int dump = 0;
    //
    CppSocket sock(PF_INET6, SOCK_RAW, IPPROTO_ICMP);
    if (!sock.isValid())
    {
        OS_LOGE("icmp socket, %s", strerror(errno));
        return ret;
    }
    //
    sock.set_reuse_addr(1);
    sock.set_reuse_port(1);
    sock.set_send_timeout(send_timeout_sec);
    sock.set_recv_timeout(recv_timeout_sec);
    if (iface && sock.bind_to_device(iface) == -1)
    {
        return ret;
    }
    //
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = PF_INET6;  // IPv4
    addr.sin6_addr   = ip6;

    //
    struct icmphdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.type             = ICMP_ECHO;
    hdr.code             = 0;
    hdr.checksum         = 0;
    hdr.un.echo.id       = 0;
    hdr.un.echo.sequence = 0;
    hdr.checksum         = icmp_checksum((uint16_t *)&hdr, sizeof(hdr));
    //
    ret = sendto(sock.get(), (char *)&hdr, sizeof(hdr), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        return ret;
    }
    uint8_t buf[1024];
    ret = sock.read_data(buf, sizeof(buf));
    if (ret == -1)
    {
        return ret;
    }
    //
    auto iphdrptr = (struct iphdr *)buf;
    //
    auto icmphdrptr = (struct icmphdr *)(buf + (iphdrptr->ihl) * 4);

    ret = -1;
    //
    switch (icmphdrptr->type)
    {
        case 3:
        {
            if (dump)
            {
                OS_LOGI(
                    "(%s)The host %s is unreachable, "
                    "ICMP_type=%d, ICMP_code=%d ",
                    iface ? iface : "all",
                    IPV6_ADDR2STR(&ip6),
                    icmphdrptr->type,
                    icmphdrptr->code);
            }
        }
        break;
        case 8:
        {
            if (dump)
            {
                OS_LOGI("(%s)The host %s is alive, ICMP_type=%d, ICMP_code=%d ",
                        iface ? iface : "all",
                        IPV6_ADDR2STR(&ip6),
                        icmphdrptr->type,
                        icmphdrptr->code);
            }
            ret = 0;
        }
        break;
        case 0:
        {
            if (dump)
            {
                OS_LOGI("(%s)The host %s is alive, ICMP_type=%d, ICMP_code=%d ",
                        iface ? iface : "all",
                        IPV6_ADDR2STR(&ip6),
                        icmphdrptr->type,
                        icmphdrptr->code);
            }
            ret = 0;
        }

        break;
        default:
        {
            if (dump)
            {
                OS_LOGI(
                    "(%s)The host %s, unknown situation, "
                    "ICMP_type=%d, ICMP_code=%d ",
                    iface ? iface : "all",
                    IPV6_ADDR2STR(&ip6),
                    icmphdrptr->type,
                    icmphdrptr->code);
            }
        }
        break;
    }

    return ret;
}



int os_net_ping4_ok(struct in_addr addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
    // try icmp ping first
    if (is_sudo(0) && os_net_icmp_ping4(addr, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 1;
    }
    // icmp packet may be blocked by firewall, try to use tcp connection
    if (os_net_tcp_ping4(addr, port, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 2;
    }

    return 0;
}

int os_net_ping6_ok(struct in6_addr addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
    // try icmp ping first
    if (is_sudo(0) && os_net_icmp_ping6(addr, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 1;
    }
    // icmp packet may be blocked by firewall, try to use tcp connection
    if (os_net_tcp_ping6(addr, port, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 2;
    }

    return 0;
}

int os_net_check_wan4()
{
    const char *testIP4 = "1.1.1.1";
    //
    int ret = os_net_ping4_ok(ipv4_str2addr(testIP4), 80, 2, 2, NULL);

    return ret > 0 ? 0 : -1;
}

int os_net_check_wan6()
{
    const char *testIP6 = "2606:4700:4700::1111";
    //
    int ret = os_net_ping6_ok(ipv6_str2addr(testIP6), 80, 2, 2, NULL);

    return ret > 0 ? 0 : -1;
}
