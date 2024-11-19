#ifndef __OS_TOOLS_NET_H__
#define __OS_TOOLS_NET_H__

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

//
#include "utils/os_tools.h"



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
    #include <cerrno>
    #include <cstdint>
    #include <list>
    #include <string>
    #include <unordered_set>

class CppSocket
{
   private:
    int sd;

   public:
    CppSocket(int domain, int type, int protocol)
    {
        sd = socket(domain, type, protocol);
    }
    ~CppSocket()
    {
        release();
    }
    bool isValid()
    {
        return sd != -1;
    }
    void release()
    {
        if (isValid())
        {
            close(sd);
            sd = -1;
        }
    }
    int get()
    {
        return sd;
    }

    static int connect_ipv4(int fd, struct in_addr ipv4_addr, uint16_t port)
    {
        int ret = -1;
        //
        struct sockaddr_in addr;
        //
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        addr.sin_addr   = ipv4_addr;
        //
        ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1)
        {
        }
        return ret;
    }
    static int connect_ipv4(int fd, const char *ipv4_str, uint16_t port)
    {
        return connect_ipv4(fd, ipv4_str2addr(ipv4_str), port);
    }
    int connect_ipv4(struct in_addr ipv4_addr, uint16_t port)
    {
        return connect_ipv4(sd, ipv4_addr, port);
    }
    int connect_ipv4(const char *ipv4_str, uint16_t port)
    {
        return connect_ipv4(sd, ipv4_str, port);
    }

    static int connect_ipv6(int fd, struct in6_addr ipv6_addr, uint16_t port)
    {
        int ret = -1;
        //
        struct sockaddr_in6 addr;
        //
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_port   = htons(port);
        addr.sin6_addr   = ipv6_addr;
        //
        ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1)
        {
        }
        return ret;
    }
    static int connect_ipv6(int fd, const char *ipv6_str, uint16_t port)
    {
        return connect_ipv6(fd, ipv6_str2addr(ipv6_str), port);
    }
    int connect_ipv6(struct in6_addr ipv6_addr, uint16_t port)
    {
        return connect_ipv6(sd, ipv6_addr, port);
    }
    int connect_ipv6(const char *ipv6_str, uint16_t port)
    {
        return connect_ipv6(sd, ipv6_str, port);
    }


    static ssize_t write_data(int sock_fd, const uint8_t *buffer, size_t buflen)
    {
        if (!buffer || buflen <= 0)
        {
            return -1;
        }
        //
        size_t offset = 0;
        //
        ssize_t ret = -1;
        while (offset < buflen)
        {
            ret = send(sock_fd, buffer + offset, buflen - offset, 0);
            if (ret < 0)
            {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    usleep(1000 * 50);
                    continue;
                }
                break;
            }
            offset += ret;
        }
        return ret < 0 ? ret : offset;
    }
    ssize_t write_data(const uint8_t *buffer, size_t buflen)
    {
        return write_data(sd, buffer, buflen);
    }

    static ssize_t read_data(int sock_fd, uint8_t *buffer, size_t buflen)
    {
        if (!buffer || buflen <= 0)
        {
            return -1;
        }
        memset(buffer, 0, buflen);
        //
        size_t offset = 0;
        //
        ssize_t ret = -1;
        while (offset < buflen)
        {
            ret = recv(sock_fd, buffer + offset, buflen - offset, 0);
            if (ret == -1)
            {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    usleep(1000 * 50);
                    continue;
                }
                break;
            }
            offset += ret;
            // break anyway
            break;
        }
        return ret < 0 ? ret : offset;
    }
    ssize_t read_data(uint8_t *buffer, size_t buflen)
    {
        return read_data(sd, buffer, buflen);
    }


    static int bind_to_ipv4_addr(int fd, struct in_addr ipv4_addr, uint16_t port)
    {
        struct sockaddr_in localaddr;
        //
        memset(&localaddr, 0, sizeof(localaddr));
        localaddr.sin_family = AF_INET;
        localaddr.sin_port   = htons(port);
        localaddr.sin_addr   = ipv4_addr;
        return bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
    }
    int bind_to_ipv4_addr(struct in_addr ipv4_addr, uint16_t port)
    {
        return bind_to_ipv4_addr(sd, ipv4_addr, port);
    }

    static int bind_to_ipv6_addr(int fd, struct in6_addr ipv6_addr, uint16_t port)
    {
        struct sockaddr_in6 localaddr;
        //
        memset(&localaddr, 0, sizeof(localaddr));
        localaddr.sin6_family = AF_INET6;
        localaddr.sin6_port   = htons(port);
        localaddr.sin6_addr   = ipv6_addr;
        return bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
    }
    int bind_to_ipv6_addr(struct in6_addr ipv6_addr, uint16_t port)
    {
        return bind_to_ipv6_addr(sd, ipv6_addr, port);
    }

    static int bind_to_device(int fd, const char *iface_name)
    {
        auto ret = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface_name, strlen(iface_name));
        if (ret == -1)
        {
            // OS_LOGE("setsockopt, SO_BINDTODEVICE %s, %s", iface_name, strerror(errno));
        }
        return ret;
    }
    int bind_to_device(const char *iface_name)
    {
        return bind_to_device(sd, iface_name);
    }

    static int set_send_timeout(int sd, int timeout_sec)
    {
        struct timeval tv;
        //
        tv.tv_sec  = timeout_sec;
        tv.tv_usec = 0;
        //
        return setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));
    }
    int set_send_timeout(int timeout_sec)
    {
        return set_send_timeout(sd, timeout_sec);
    }

    static int set_recv_timeout(int sd, int timeout_sec)
    {
        struct timeval tv;
        //
        tv.tv_sec  = timeout_sec;
        tv.tv_usec = 0;
        //
        return setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    }
    int set_recv_timeout(int timeout_sec)
    {
        return set_recv_timeout(sd, timeout_sec);
    }


    static int set_reuse_addr(int sd, bool reuse)
    {
        int opt = reuse ? 1 : 0;
        return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    int set_reuse_addr(bool reuse)
    {
        return set_reuse_addr(sd, reuse);
    }

    static int set_reuse_port(int sd, bool reuse)
    {
        int opt = reuse ? 1 : 0;
        return setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    }
    int set_reuse_port(bool reuse)
    {
        return set_reuse_port(sd, reuse);
    }

    /**
     * @brief \ref https://man7.org/linux/man-pages/man7/tcp.7.html
     * - TCP_NODELAY
                  If set, disable the Nagle algorithm.  This means that
                  segments are always sent as soon as possible, even if
                  there is only a small amount of data.  When not set, data
                  is buffered until there is a sufficient amount to send
                  out, thereby avoiding the frequent sending of small
                  packets, which results in poor utilization of the network.
                  This option is overridden by TCP_CORK; however, setting
                  this option forces an explicit flush of pending output,
                  even if TCP_CORK is currently set.
     *
     * @param sd
     * @param enable
     * @return int
     */
    static int set_tcp_nodelay(int sd, bool enable)
    {
        int opt = enable ? 1 : 0;
        return setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    }
    int set_tcp_nodelay(bool enable)
    {
        return set_tcp_nodelay(sd, enable);
    }

    /**
     * @brief \ref https://man7.org/linux/man-pages/man7/tcp.7.html
     *
     * @param sd : A descriptor identifying the socket.
     * @param keepalive :
     * The setsockopt function called with the SO_KEEPALIVE socket option allows an application to enable keep-alive packets for a socket connection.
     * The SO_KEEPALIVE option for a socket is disabled (set to FALSE) by default.
     * @param cnt :     TCP_KEEPCNT (since Linux 2.4)
     *           The maximum number of keepalive probes TCP should send
     *           before dropping the connection.  This option should not be
     *           used in code intended to be portable.
     * - default value : 9
     * @param idle :          TCP_KEEPIDLE (since Linux 2.4)
     *           The time (in seconds) the connection needs to remain idle
     *           before TCP starts sending keepalive probes, if the socket
     *           option SO_KEEPALIVE has been set on this socket.  This
     *           option should not be used in code intended to be portable.
     * - default value : 7200
     * @param interval :          TCP_KEEPINTVL (since Linux 2.4)
     *           The time (in seconds) between individual keepalive probes.
     *           This option should not be used in code intended to be
     *           portable.
     * - default value : 75
     */
    static int set_tcp_keep(int sd, bool keepalive = false, int cnt = -1, int idle = -1, int interval = -1)
    {
        {
            int opt = keepalive ? 1 : 0;
            setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
        }
        if (cnt >= 0)
        {
            int opt = cnt;
            setsockopt(sd, IPPROTO_TCP, TCP_KEEPCNT, &opt, sizeof(opt));
        }
        if (idle >= 0)
        {
            int opt = idle;
            setsockopt(sd, IPPROTO_TCP, TCP_KEEPIDLE, &opt, sizeof(opt));
        }
        if (interval >= 0)
        {
            int opt = interval;
            setsockopt(sd, IPPROTO_TCP, TCP_KEEPINTVL, &opt, sizeof(opt));
        }
        return 0;
    }
    int set_tcp_keep(bool keepalive = false, int cnt = -1, int idle = -1, int interval = -1)
    {
        return set_tcp_keep(sd, keepalive, cnt, idle, interval);
    }
};

class CppGetIfaces
{
   private:
    struct ifaddrs *ifaddr;

   public:
    CppGetIfaces()
    {
        ifaddr = NULL;
        //
        if (getifaddrs(&ifaddr) == -1)
        {
            OS_LOGE("getifaddrs, %s", strerror(errno));
        }
    }
    ~CppGetIfaces()
    {
        release();
    }

    void release()
    {
        if (ifaddr)
        {
            freeifaddrs(ifaddr);
            ifaddr = NULL;
        }
    }

    struct ifaddrs *get()
    {
        return ifaddr;
    }

    std::unordered_set<std::string> getIfaces()
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
};

int os_net_nslookup(const char *hostname, std::list<struct in_addr> &ipv4_addrs, std::list<struct in6_addr> &ipv6_addrs);



#endif



#endif  // __OS_TOOLS_NET_H__
