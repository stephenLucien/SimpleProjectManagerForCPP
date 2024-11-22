#ifndef OS_TOOLS_NET_SOCKET_HPP
#define OS_TOOLS_NET_SOCKET_HPP


#include <cerrno>
#include <cstdint>
#include <cstdio>

class CppSocket
{
   private:
    int sd;

   public:
    CppSocket(int domain, int type, int protocol);
    ~CppSocket();
    bool isValid();
    void release();
    int  get();

    static int connect_ipv4(int fd, struct in_addr ipv4_addr, uint16_t port);
    static int connect_ipv4(int fd, const char *ipv4_str, uint16_t port);
    //
    int connect_ipv4(struct in_addr ipv4_addr, uint16_t port);
    int connect_ipv4(const char *ipv4_str, uint16_t port);

    static int connect_ipv6(int fd, struct in6_addr ipv6_addr, uint16_t port);
    static int connect_ipv6(int fd, const char *ipv6_str, uint16_t port);
    //
    int connect_ipv6(struct in6_addr ipv6_addr, uint16_t port);
    int connect_ipv6(const char *ipv6_str, uint16_t port);


    static ssize_t write_data(int sock_fd, const uint8_t *buffer, size_t buflen);
    //
    ssize_t write_data(const uint8_t *buffer, size_t buflen);

    static ssize_t read_data(int sock_fd, uint8_t *buffer, size_t buflen);
    //
    ssize_t read_data(uint8_t *buffer, size_t buflen);


    static int bind_to_ipv4_addr(int fd, struct in_addr ipv4_addr, uint16_t port);
    //
    int bind_to_ipv4_addr(struct in_addr ipv4_addr, uint16_t port);

    static int bind_to_ipv6_addr(int fd, struct in6_addr ipv6_addr, uint16_t port);
    //
    int bind_to_ipv6_addr(struct in6_addr ipv6_addr, uint16_t port);

    static int bind_to_device(int fd, const char *iface_name);
    //
    int bind_to_device(const char *iface_name);

    static int set_send_timeout(int sd, int timeout_sec);
    //
    int set_send_timeout(int timeout_sec);

    static int set_recv_timeout(int sd, int timeout_sec);
    //
    int set_recv_timeout(int timeout_sec);


    static int set_reuse_addr(int sd, bool reuse);
    //
    int set_reuse_addr(bool reuse);

    static int set_reuse_port(int sd, bool reuse);
    //
    int set_reuse_port(bool reuse);

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
    static int set_tcp_nodelay(int sd, bool enable);
    //
    int set_tcp_nodelay(bool enable);

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
    static int set_tcp_keep(int sd, bool keepalive = false, int cnt = -1, int idle = -1, int interval = -1);
    //
    int set_tcp_keep(bool keepalive = false, int cnt = -1, int idle = -1, int interval = -1);
};

#endif /* OS_TOOLS_NET_SOCKET_HPP */
