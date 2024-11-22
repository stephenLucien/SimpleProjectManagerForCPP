#ifndef __OS_TOOLS_NET_H__
#define __OS_TOOLS_NET_H__

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#ifdef __cplusplus
extern "C" {
#endif

static inline void set_invalid_ip(void *p, size_t len)
{
    memset(p, 0xFF, len);
}
#define INVALID_IP4(ip) set_invalid_ip((void *)(ip), sizeof(struct in_addr))
#define INVALID_IP6(ip) set_invalid_ip((void *)(ip), sizeof(struct in6_addr))

static inline int is_invalid_ip(void *p, size_t len)
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
#define IS_INVALID_IP4(ip) is_invalid_ip((void *)(ip), sizeof(struct in_addr))
#define IS_INVALID_IP6(ip) is_invalid_ip((void *)(ip), sizeof(struct in6_addr))

static inline const char *ipv4_addr2str(struct in_addr *ip, char *buf, size_t bufsz)
{
    int sa_family = AF_INET;
    //
    memset(buf, 0, bufsz);
    //
    return inet_ntop(sa_family, (void *)ip, buf, bufsz);
}
#define IPV4_ADDR2STR(ip) ipv4_addr2str((struct in_addr *)(ip), (char *)__builtin_alloca(128), 128)

static inline const char *ipv6_addr2str(struct in6_addr *ip6, char *buf, size_t bufsz)
{
    int sa_family = AF_INET6;
    //
    memset(buf, 0, bufsz);
    //
    return inet_ntop(sa_family, (void *)ip6, buf, bufsz);
}
#define IPV6_ADDR2STR(ip) ipv6_addr2str((struct in6_addr *)(ip), (char *)__builtin_alloca(128), 128)

static inline struct in_addr ipv4_str2addr(const char *ipstr)
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

static inline struct in6_addr ipv6_str2addr(const char *ip6str)
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


int os_net_get_local_ipv4_addr(const char *iface, struct in_addr &ip4);

int os_net_get_local_ipv6_addr(const char *iface, struct in6_addr &ip6);

int socket_bind_to_ipv4_addr(int fd, struct in_addr ipv4_addr, uint16_t port);

int socket_bind_to_ipv6_addr(int fd, struct in6_addr ipv6_addr, uint16_t port);

int socket_bind_to_device(int fd, const char *iface_name);

struct in_addr os_net_nslookup4(const char *hostname);

struct in6_addr os_net_nslookup6(const char *hostname);

int os_net_tcp_ping4(struct in_addr ipv4_addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface);

int os_net_tcp_ping6(struct in6_addr ipv6_addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface);

int os_net_icmp_ping4(struct in_addr ip4, int send_timeout_sec, int recv_timeout_sec, const char *iface);

int os_net_icmp_ping6(struct in6_addr ip6, int send_timeout_sec, int recv_timeout_sec, const char *iface);

int os_net_ping4_ok(struct in_addr addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface);

int os_net_ping6_ok(struct in6_addr addr, uint16_t port, int send_timeout_sec, int recv_timeout_sec, const char *iface);

int os_net_check_wan4();

int os_net_check_wan6();

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
    #include <list>

int os_net_nslookup(const char *hostname, std::list<struct in_addr> &ipv4_addrs, std::list<struct in6_addr> &ipv6_addrs);



#endif



#endif  // __OS_TOOLS_NET_H__
