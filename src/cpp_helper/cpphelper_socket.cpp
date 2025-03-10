#include "cpp_helper/cpphelper_socket.hpp"

//
#include "utils/os_tools_net.h"

//
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>


CppSocket::CppSocket(int domain, int type, int protocol)
{
    sd = socket(domain, type, protocol);
}
CppSocket::~CppSocket()
{
    release();
}
bool CppSocket::isValid()
{
    return sd != -1;
}
void CppSocket::release()
{
    if (isValid())
    {
        close(sd);
        sd = -1;
    }
}
int CppSocket::get()
{
    return sd;
}
int CppSocket::connect_ipv4(int fd, struct in_addr ipv4_addr, uint16_t port)
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
int CppSocket::connect_ipv4(int fd, const char *ipv4_str, uint16_t port)
{
    return connect_ipv4(fd, ipv4_str2addr(ipv4_str), port);
}
int CppSocket::connect_ipv4(struct in_addr ipv4_addr, uint16_t port)
{
    return connect_ipv4(sd, ipv4_addr, port);
}
int CppSocket::connect_ipv4(const char *ipv4_str, uint16_t port)
{
    return connect_ipv4(sd, ipv4_str, port);
}
int CppSocket::connect_ipv6(int fd, struct in6_addr ipv6_addr, uint16_t port)
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
int CppSocket::connect_ipv6(int fd, const char *ipv6_str, uint16_t port)
{
    return connect_ipv6(fd, ipv6_str2addr(ipv6_str), port);
}
int CppSocket::connect_ipv6(struct in6_addr ipv6_addr, uint16_t port)
{
    return connect_ipv6(sd, ipv6_addr, port);
}
int CppSocket::connect_ipv6(const char *ipv6_str, uint16_t port)
{
    return connect_ipv6(sd, ipv6_str, port);
}
ssize_t CppSocket::write_data(int sock_fd, const uint8_t *buffer, size_t buflen)
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
ssize_t CppSocket::write_data(const uint8_t *buffer, size_t buflen)
{
    return write_data(sd, buffer, buflen);
}
ssize_t CppSocket::read_data(int sock_fd, uint8_t *buffer, size_t buflen)
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
ssize_t CppSocket::read_data(uint8_t *buffer, size_t buflen)
{
    return read_data(sd, buffer, buflen);
}
int CppSocket::bind_to_ipv4_addr(int fd, struct in_addr ipv4_addr, uint16_t port)
{
    struct sockaddr_in localaddr;
    //
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_port   = htons(port);
    localaddr.sin_addr   = ipv4_addr;
    return bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
}
int CppSocket::bind_to_ipv4_addr(struct in_addr ipv4_addr, uint16_t port)
{
    return bind_to_ipv4_addr(sd, ipv4_addr, port);
}
int CppSocket::bind_to_ipv6_addr(int fd, struct in6_addr ipv6_addr, uint16_t port)
{
    struct sockaddr_in6 localaddr;
    //
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin6_family = AF_INET6;
    localaddr.sin6_port   = htons(port);
    localaddr.sin6_addr   = ipv6_addr;
    return bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
}
int CppSocket::bind_to_ipv6_addr(struct in6_addr ipv6_addr, uint16_t port)
{
    return bind_to_ipv6_addr(sd, ipv6_addr, port);
}
int CppSocket::bind_to_device(int fd, const char *iface_name)
{
    auto ret = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface_name, strlen(iface_name));
    if (ret == -1)
    {
        // OS_LOGE("setsockopt, SO_BINDTODEVICE %s, %s", iface_name, strerror(errno));
    }
    return ret;
}
int CppSocket::bind_to_device(const char *iface_name)
{
    return bind_to_device(sd, iface_name);
}
int CppSocket::set_send_timeout(int sd, int timeout_sec, int timeout_us)
{
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    //
    if (timeout_sec >= 0)
    {
        tv.tv_sec = timeout_sec;
    }
    if (timeout_us >= 0)
    {
        tv.tv_usec = timeout_us;
    }
    //
    return setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));
}
int CppSocket::set_send_timeout(int timeout_sec, int timeout_us)
{
    return set_send_timeout(sd, timeout_sec, timeout_us);
}
int CppSocket::set_recv_timeout(int sd, int timeout_sec, int timeout_us)
{
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    //
    if (timeout_sec >= 0)
    {
        tv.tv_sec = timeout_sec;
    }
    if (timeout_us >= 0)
    {
        tv.tv_usec = timeout_us;
    }
    //
    return setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
}
int CppSocket::set_recv_timeout(int timeout_sec, int timeout_us)
{
    return set_recv_timeout(sd, timeout_sec, timeout_us);
}
int CppSocket::set_reuse_addr(int sd, bool reuse)
{
    int opt = reuse ? 1 : 0;
    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}
int CppSocket::set_reuse_addr(bool reuse)
{
    return set_reuse_addr(sd, reuse);
}
int CppSocket::set_reuse_port(int sd, bool reuse)
{
    int opt = reuse ? 1 : 0;
    return setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}
int CppSocket::set_reuse_port(bool reuse)
{
    return set_reuse_port(sd, reuse);
}
int CppSocket::set_tcp_nodelay(int sd, bool enable)
{
    int opt = enable ? 1 : 0;
    return setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}
int CppSocket::set_tcp_nodelay(bool enable)
{
    return set_tcp_nodelay(sd, enable);
}
int CppSocket::set_tcp_keep(int sd, bool keepalive, int cnt, int idle, int interval)
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
int CppSocket::set_tcp_keep(bool keepalive, int cnt, int idle, int interval)
{
    return set_tcp_keep(sd, keepalive, cnt, idle, interval);
}
