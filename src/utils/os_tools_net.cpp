#include "os_tools_net.h"

//
#include "os_tools_log.h"
#include "cpp_helper/cpphelper_socket.hpp"
#include "os_tools_system.h"
#include "cstring_proc.h"

//
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <list>

void set_invalid_ip(void *p, size_t len)
{
    memset(p, 0xFF, len);
}

int is_invalid_ip(void *p, size_t len)
{
    uint8_t *ptr = (uint8_t *)p;
    for (; len; --len, ++ptr)
    {
        if (ptr[0] != 0xFF)
        {
            return 1;
        }
    }
    return 0;
}

const char *ipv4_addr2str(struct in_addr *ip, char *buf, size_t bufsz)
{
    int sa_family = AF_INET;
    //
    memset(buf, 0, bufsz);
    //
    return inet_ntop(sa_family, (void *)ip, buf, bufsz);
}

const char *ipv6_addr2str(struct in6_addr *ip6, char *buf, size_t bufsz)
{
    int sa_family = AF_INET6;
    //
    memset(buf, 0, bufsz);
    //
    return inet_ntop(sa_family, (void *)ip6, buf, bufsz);
}

struct in_addr ipv4_str2addr(const char *ipstr)
{
    struct in_addr sin_addr;
    INVALID_IP4(&sin_addr);
    //
    int ret = -1;
    //
    if (ipstr)
    {
        ret = inet_pton(PF_INET, ipstr, &sin_addr);
    }
    if (ret < 0)
    {
        // err
    }
    return sin_addr;
}

struct in6_addr ipv6_str2addr(const char *ip6str)
{
    struct in6_addr sin_addr;
    INVALID_IP6(&sin_addr);
    //
    int ret = -1;
    //
    if (ip6str)
    {
        ret = inet_pton(PF_INET6, ip6str, &sin_addr);
    }
    if (ret < 0)
    {
        // err
    }
    return sin_addr;
}



int os_net_iface_get_flag(const char *iface, int *flags)
{
    int ret = -1;
    //
    if (!iface)
    {
        return ret;
    }
    //
    CppSocket sfd(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (!sfd.isValid())
    {
        return ret;
    }
    //
    int fd = sfd.get();
    //
    struct ifreq s;
    //
    memset(&s, 0, sizeof(s));
    //
    strncpy(s.ifr_name, iface, IFNAMSIZ - 1);
    //
    if (0 != ioctl(fd, SIOCGIFFLAGS, &s))
    {
        OS_LOGE("ioctl SIOCGIFFLAGS, err: %s", strerror(errno));
        return ret;
    }
    if (flags)
    {
        *flags = s.ifr_flags;
    }

    return 0;
}

int os_net_iface_is_up(const char *iface)
{
    int ret = 0;
    //
    if (!iface)
    {
        return ret;
    }
    //
    int flags = 0;
    //
    if (os_net_iface_get_flag(iface, &flags) != 0)
    {
        return ret;
    }
    OS_LOGV("flags: %d(0x%04X)", flags, flags);
    //
    if (flags & IFF_UP)
    {
        ret = 1;
    }
    return ret;
}

int os_net_iface_get_hwaddr(const char *iface, uint8_t mac[IFHWADDRLEN])
{
    int ret = -1;
    //
    memset(mac, 0, IFHWADDRLEN);
    //
    if (!iface)
    {
        return ret;
    }
    //
    CppSocket sfd(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (!sfd.isValid())
    {
        return ret;
    }
    //
    int fd = sfd.get();
    //
    struct ifreq s;
    //
    memset(&s, 0, sizeof(s));
    //
    strncpy(s.ifr_name, iface, IFNAMSIZ - 1);
    //
    if (0 != ioctl(fd, SIOCGIFHWADDR, &s))
    {
        OS_LOGE("ioctl SIOCGIFHWADDR, err: %s", strerror(errno));
        return ret;
    }
    memcpy(mac, s.ifr_addr.sa_data, IFHWADDRLEN);

    return 0;
}

char *os_net_iface_get_hwaddr_str(const char *iface, char *buf, size_t bufsz, int upper_case, int revert)
{
    if (!buf || bufsz < IFHWADDRLEN * 2)
    {
        return buf;
    }
    uint8_t mac[IFHWADDRLEN];
    memset(buf, 0, bufsz);
    if (os_net_iface_get_hwaddr(iface, mac) == 0)
    {
        if (revert)
        {
            r_byteArray2hexString(buf, bufsz, mac, IFHWADDRLEN, upper_case);
        } else
        {
            byteArray2hexString(buf, bufsz, mac, IFHWADDRLEN, upper_case);
        }
    }
    return buf;
}

int os_net_iface_set_hwaddr(const char *iface, uint8_t mac[IFHWADDRLEN])
{
    int ret = -1;
    //
    if (!iface)
    {
        OS_LOGE("");
        return ret;
    }
    OS_LOGI("iface(%s), hw_ether=%s", iface, OS_LOG_HEXDUMP(mac, IFHWADDRLEN));
    //
    CppSocket sfd(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (!sfd.isValid())
    {
        OS_LOGE("");
        return ret;
    }
    //
    int fd = sfd.get();
    //
    struct ifreq s;
    //
    memset(&s, 0, sizeof(s));
    //
    strncpy(s.ifr_name, iface, IFNAMSIZ - 1);
    //
    if (0 != ioctl(fd, SIOCGIFHWADDR, &s))
    {
        OS_LOGE("ioctl SIOCGIFHWADDR, err: %s", strerror(errno));
        return ret;
    }
    //
    memcpy(s.ifr_addr.sa_data, mac, IFHWADDRLEN);
    //
    if (0 != ioctl(fd, SIOCSIFHWADDR, &s))
    {
        OS_LOGE("ioctl SIOCSIFHWADDR, err: %s", strerror(errno));
        return ret;
    }

    return 0;
}

int os_net_iface_set_hwaddr_str(const char *iface, const char *mac_str, int revert)
{
    int ret = -1;
    //
    if (!iface || !mac_str)
    {
        OS_LOGE("");
        return ret;
    }
    //
    char tmpmac_str[IFHWADDRLEN * 2 + 1];
    memset(tmpmac_str, 0, sizeof(tmpmac_str));
    int mac_str_offset = 0;
    //
    for (int i = 0; i < (int)strlen(mac_str) && mac_str_offset < IFHWADDRLEN * 2; ++i)
    {
        if ((mac_str[i] >= '0' && mac_str[i] <= '9') || (mac_str[i] >= 'a' && mac_str[i] <= 'z') || (mac_str[i] >= 'A' && mac_str[i] <= 'Z'))
        {
            tmpmac_str[mac_str_offset++] = mac_str[i];
        }
    }
    if (mac_str_offset != IFHWADDRLEN * 2)
    {
        OS_LOGE("");
        return ret;
    }
    //
    uint8_t mac[IFHWADDRLEN];
    memset(mac, 0, sizeof(mac));
    hexString2byteArray(mac, sizeof(mac), tmpmac_str, IFHWADDRLEN * 2);
    if (revert)
    {
        revert_byte_array(mac, sizeof(mac));
    }
    //
    ret = os_net_iface_set_hwaddr(iface, mac);

    return ret;
}

int os_net_get_local_ipv4_addr(const char *iface, struct in_addr *p_ip4)
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
        OS_LOGE("ioctl SIOCGIFADDR, err: %s", strerror(errno));
        return -1;
    }

    *p_ip4 = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    return 0;
}

const char *os_net_get_local_ipv4_addr_str(const char *iface, char *buf, size_t bufsz)
{
    // 255.255.255.255
    if (!buf || bufsz < 16)
    {
        return buf;
    }
    struct in_addr ip4;
    os_net_get_local_ipv4_addr(iface, &ip4);
    //
    return ipv4_addr2str(&ip4, buf, bufsz);
}

int os_net_set_local_ipv4_addr(const char *iface, struct in_addr ip4)
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
    //
    // if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
    // {
    //     OS_LOGE("ioctl SIOCGIFADDR, err: %s", strerror(errno));
    //     return -1;
    // }
    //
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = ip4;
    //
    if (ioctl(fd, SIOCSIFADDR, &ifr) != 0)
    {
        OS_LOGE("ioctl SIOCSIFADDR, err: %s", strerror(errno));
        return -1;
    }

    return 0;
}

#if 0
int os_net_get_local_ipv6_addr(const char *iface, struct in6_addr *p_ip6)
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
        OS_LOGE("ioctl SIOCGIFADDR, err: %s", strerror(errno));
        return -1;
    }

    *p_ip6 = ((struct sockaddr_in6 *)&ifr.ifr_addr)->sin6_addr;

    return 0;
}

const char *os_net_get_local_ipv6_addr_str(const char *iface, char *buf, size_t bufsz)
{
    // A0A1:A2A3:A4A5:A6A7:A8A9:A0A1:A2A3:A4A5
    if (!buf || bufsz < 40)
    {
        return buf;
    }
    struct in6_addr ip6;
    os_net_get_local_ipv6_addr(iface, &ip6);
    //
    return ipv6_addr2str(&ip6, buf, bufsz);
}

int os_net_set_local_ipv6_addr(const char *iface, struct in6_addr ip6)
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
    //
    // if (ioctl(sock.get(), SIOCGIFADDR, &ifr) != 0)
    // {
    //     OS_LOGE("ioctl SIOCGIFADDR, err: %s", strerror(errno));
    //     return -1;
    // }
    //
    ((struct sockaddr_in6 *)&ifr.ifr_addr)->sin6_addr = ip6;
    //
    if (ioctl(sock.get(), SIOCSIFADDR, &ifr) != 0)
    {
        OS_LOGE("ioctl SIOCSIFADDR, err: %s", strerror(errno));
        return -1;
    }

    return 0;
}
#endif

int os_net_get_local_ipv4_netmask_addr(const char *iface, struct in_addr *p_ip4)
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

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) != 0)
    {
        OS_LOGE("ioctl SIOCGIFNETMASK, err: %s", strerror(errno));
        return -1;
    }

    *p_ip4 = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    return 0;
}


const char *os_net_get_local_ipv4_netmask_addr_str(const char *iface, char *buf, size_t bufsz)
{
    // 255.255.255.255
    if (!buf || bufsz < 16)
    {
        return buf;
    }
    struct in_addr ip4;
    os_net_get_local_ipv4_netmask_addr(iface, &ip4);
    //
    return ipv4_addr2str(&ip4, buf, bufsz);
}

int os_net_set_local_ipv4_netmask_addr(const char *iface, struct in_addr ip4)
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
    //
    // if (ioctl(fd, SIOCGIFNETMASK, &ifr) != 0)
    // {
    //     OS_LOGE("ioctl SIOCGIFNETMASK, err: %s", strerror(errno));
    //     return -1;
    // }
    //
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = ip4;
    //
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0)
    {
        OS_LOGE("ioctl SIOCSIFNETMASK, err: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int os_net_get_local_ipv4_broadcast_addr(const char *iface, struct in_addr *p_ip4)
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

    if (ioctl(fd, SIOCGIFBRDADDR, &ifr) != 0)
    {
        OS_LOGE("ioctl SIOCGIFBRDADDR, err: %s", strerror(errno));
        return -1;
    }

    *p_ip4 = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    return 0;
}


const char *os_net_get_local_ipv4_broadcast_addr_str(const char *iface, char *buf, size_t bufsz)
{
    // 255.255.255.255
    if (!buf || bufsz < 16)
    {
        return buf;
    }
    struct in_addr ip4;
    os_net_get_local_ipv4_broadcast_addr(iface, &ip4);
    //
    return ipv4_addr2str(&ip4, buf, bufsz);
}

int os_net_set_local_ipv4_broadcast_addr(const char *iface, struct in_addr ip4)
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
    //
    // if (ioctl(fd, SIOCGIFBRDADDR, &ifr) != 0)
    // {
    //     OS_LOGE("ioctl SIOCGIFBRDADDR, err: %s", strerror(errno));
    //     return -1;
    // }
    //
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = ip4;
    //
    if (ioctl(fd, SIOCSIFBRDADDR, &ifr) != 0)
    {
        OS_LOGE("ioctl SIOCSIFBRDADDR, err: %s", strerror(errno));
        return -1;
    }

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
    CppSocket sock(PF_INET6, SOCK_STREAM, IPPROTO_IP);
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

/**
 * @brief \ref https://en.wikipedia.org/wiki/Internet_checksum
 *
 * @param buf
 * @param bufsz
 * @return uint16_t
 */
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
    int dump = 1;
    //
    CppSocket sock(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (!sock.isValid())
    {
        if (dump)
        {
            OS_LOGE("icmp socket, %s", strerror(errno));
        }
        return ret;
    }
    //
    sock.set_reuse_addr(1);
    sock.set_reuse_port(1);
    sock.set_send_timeout(send_timeout_sec);
    sock.set_recv_timeout(recv_timeout_sec);
    if (iface && sock.bind_to_device(iface) == -1)
    {
        if (dump)
        {
            OS_LOGE("bind_to_device, %s", strerror(errno));
        }
        return ret;
    }
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = PF_INET;  // IPv4
    addr.sin_addr   = ip4;

    /**
     * @brief \ref https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
     *
     */
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
        if (dump)
        {
            OS_LOGE("sendto, %s", strerror(errno));
        }
        return ret;
    }
    uint8_t buf[1024];
    memset(buf, 0, sizeof(buf));
    ret = sock.read_data(buf, sizeof(buf));
    if (ret == -1)
    {
        if (dump)
        {
            OS_LOGE("read_data, %s", strerror(errno));
        }
        return ret;
    }
#if 1
    if (dump)
    {
        OS_LOGV("buf:%s", OS_LOG_HEXDUMP(buf, ret));
    }
#endif

    ret = -1;
    //
    auto iphdrptr = (struct iphdr *)buf;

    /**
     * @brief
     * \ref https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
     *
     */
    if (iphdrptr->protocol != IPPROTO_ICMP)
    {
        if (dump)
        {
            OS_LOGE("protocol %02X", iphdrptr->protocol);
        }
        return ret;
    }
    /**
     * @brief there might be 'The options' after daddr.
     * use ihl(int 32bit word) to calculate the offset of ICMP header
     */
    auto icmphdrptr = (struct icmphdr *)(buf + (iphdrptr->ihl) * 4);

    //
    switch (icmphdrptr->type)
    {
        case ICMP_DEST_UNREACH:
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
        case ICMP_ECHO:
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
        case ICMP_ECHOREPLY:
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
    int dump = 1;
    //
    CppSocket sock(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (!sock.isValid())
    {
        if (dump)
        {
            OS_LOGE("icmp socket, %s", strerror(errno));
        }
        return ret;
    }
    //
    sock.set_reuse_addr(1);
    sock.set_reuse_port(1);
    sock.set_send_timeout(send_timeout_sec);
    sock.set_recv_timeout(recv_timeout_sec);
    if (iface && sock.bind_to_device(iface) == -1)
    {
        if (dump)
        {
            OS_LOGE("bind_to_device, %s", strerror(errno));
        }
        return ret;
    }
    /**
     * @brief \ref https://www.rfc-editor.org/rfc/rfc3493
     *
     */
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = PF_INET6;  // IPv6
    addr.sin6_addr   = ip6;

    /**
     * @brief \ref https://en.wikipedia.org/wiki/ICMPv6
     *
     */
    struct icmp6_hdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.icmp6_type  = ICMP6_ECHO_REQUEST;
    hdr.icmp6_code  = 0;
    hdr.icmp6_cksum = icmp_checksum((uint16_t *)&hdr, sizeof(hdr));
    //
    ret = sendto(sock.get(), (char *)&hdr, sizeof(hdr), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        if (dump)
        {
            OS_LOGE("sendto, %s", strerror(errno));
        }
        return ret;
    }
    uint8_t buf[1024];
    memset(buf, 0, sizeof(buf));
    ret = sock.read_data(buf, sizeof(buf));
    if (ret == -1)
    {
        if (dump)
        {
            OS_LOGE("read_data, %s", strerror(errno));
        }
        return ret;
    }
#if 1
    if (dump)
    {
        OS_LOGV("buf:%s", OS_LOG_HEXDUMP(buf, ret));
    }
#endif

    ret = -1;
#if 1
    auto icmphdrptr = (struct icmp6_hdr *)(buf);
#else
    /**
     * @brief \ref https://www.rfc-editor.org/rfc/rfc3542#section-2.2
     *
     */
    auto iphdrptr = (struct ip6_hdr *)buf;
    if (iphdrptr->ip6_ctlun.ip6_un1.ip6_un1_nxt != IPPROTO_ICMPV6)
    {
        if (dump)
        {
            OS_LOGE("header next, %02X", iphdrptr->ip6_ctlun.ip6_un1.ip6_un1_nxt);
        }
        return ret;
    }
    //
    auto icmphdrptr = (struct icmp6_hdr *)(buf + sizeof(struct ip6_hdr));
#endif

    ret = -1;
    //
    switch (icmphdrptr->icmp6_type)
    {
        case ICMP6_DST_UNREACH_ADDR:
        {
            if (dump)
            {
                OS_LOGI(
                    "(%s)The host %s is unreachable, "
                    "ICMP_type=%d, ICMP_code=%d ",
                    iface ? iface : "all",
                    IPV6_ADDR2STR(&ip6),
                    icmphdrptr->icmp6_type,
                    icmphdrptr->icmp6_code);
            }
        }
        break;
        case ICMP6_ECHO_REQUEST:
        {
            if (dump)
            {
                OS_LOGI("(%s)The host %s is alive, ICMP_type=%d, ICMP_code=%d ",
                        iface ? iface : "all",
                        IPV6_ADDR2STR(&ip6),
                        icmphdrptr->icmp6_type,
                        icmphdrptr->icmp6_code);
            }
            ret = 0;
        }
        break;
        case ICMP6_ECHO_REPLY:
        {
            if (dump)
            {
                OS_LOGI("(%s)The host %s is alive, ICMP_type=%d, ICMP_code=%d ",
                        iface ? iface : "all",
                        IPV6_ADDR2STR(&ip6),
                        icmphdrptr->icmp6_type,
                        icmphdrptr->icmp6_code);
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
                    icmphdrptr->icmp6_type,
                    icmphdrptr->icmp6_code);
            }
        }
        break;
    }

    return ret;
}



int os_net_ping4_ok(struct in_addr addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
#if 0
    // try icmp ping first
    if (is_sudo(0) && os_net_icmp_ping4(addr, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 1;
    }
#endif
    // icmp packet may be blocked by firewall, try to use tcp connection
    if (os_net_tcp_ping4(addr, port, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 2;
    }

    return 0;
}

int os_net_ping6_ok(struct in6_addr addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface)
{
#if 0
    // try icmp ping first
    if (is_sudo(0) && os_net_icmp_ping6(addr, send_timeout_sec, recv_timeout_sec, iface) == 0)
    {
        return 1;
    }
#endif
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
