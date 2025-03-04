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

void set_invalid_ip(void *p, size_t len);
#define INVALID_IP4(ip) set_invalid_ip((void *)(ip), sizeof(struct in_addr))
#define INVALID_IP6(ip) set_invalid_ip((void *)(ip), sizeof(struct in6_addr))

int is_invalid_ip(void *p, size_t len);
#define IS_INVALID_IP4(ip) is_invalid_ip((void *)(ip), sizeof(struct in_addr))
#define IS_INVALID_IP6(ip) is_invalid_ip((void *)(ip), sizeof(struct in6_addr))

const char *ipv4_addr2str(struct in_addr *ip, char *buf, size_t bufsz);
#define IPV4_ADDR2STR(ip) ipv4_addr2str((struct in_addr *)(ip), (char *)__builtin_alloca(128), 128)

const char *ipv6_addr2str(struct in6_addr *ip6, char *buf, size_t bufsz);
#define IPV6_ADDR2STR(ip) ipv6_addr2str((struct in6_addr *)(ip), (char *)__builtin_alloca(128), 128)

struct in_addr ipv4_str2addr(const char *ipstr);

struct in6_addr ipv6_str2addr(const char *ip6str);

//
int os_net_iface_get_flag(const char *iface, int *flags);
//
int os_net_iface_is_up(const char *iface);
//
int os_net_iface_get_hwaddr(const char *iface, uint8_t mac[6]);
//
char *os_net_iface_get_hwaddr_str(const char *iface, char *buf, size_t bufsz, int upper_case = 0, int revert = 0);
#define OS_NET_IFACE_GET_MAC_STR(iface) os_net_iface_get_hwaddr_str(iface, (char *)__builtin_alloca(16), 16, 0, 0)
//
int os_net_iface_set_hwaddr(const char *iface, uint8_t mac[6]);
//
int os_net_iface_set_hwaddr_str(const char *iface, const char *mac_str, int revert = 0);

int os_net_get_local_ipv4_addr(const char *iface, struct in_addr *p_ip4);

const char *os_net_get_local_ipv4_addr_str(const char *iface, char *buf, size_t bufsz);
#define OS_NET_IFACE_GET_ADDR(iface) os_net_get_local_ipv4_addr_str(iface, (char *)__builtin_alloca(128), 128)

int os_net_set_local_ipv4_addr(const char *iface, struct in_addr ip4);

int os_net_get_local_ipv4_netmask_addr(const char *iface, struct in_addr *p_ip4);

const char *os_net_get_local_ipv4_netmask_addr_str(const char *iface, char *buf, size_t bufsz);
#define OS_NET_IFACE_GET_NETMASK(iface) os_net_get_local_ipv4_netmask_addr_str(iface, (char *)__builtin_alloca(128), 128)


int os_net_set_local_ipv4_netmask_addr(const char *iface, struct in_addr ip4);

int os_net_get_local_ipv4_broadcast_addr(const char *iface, struct in_addr *p_ip4);

const char *os_net_get_local_ipv4_broadcast_addr_str(const char *iface, char *buf, size_t bufsz);
#define OS_NET_IFACE_GET_BRADDR(iface) os_net_get_local_ipv4_broadcast_addr_str(iface, (char *)__builtin_alloca(128), 128)


int os_net_set_local_ipv4_broadcast_addr(const char *iface, struct in_addr ip4);


int os_net_iface_get_ipv4_gwaddr(const char *iface, struct in_addr *p_ip4, int recv_timeout_ms = -1, int send_timeout_ms = -1);

char *os_net_iface_get_ipv4_gwaddr_str(const char *iface, char *buf, size_t bufsz);
#define OS_NET_IFACE_GET_GWADDR(iface) os_net_iface_get_ipv4_gwaddr_str(iface, (char *)__builtin_alloca(128), 128)


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
