#include "os_tools_net_sntp.h"

//
#include <cstdint>
#include <cstdio>
#include "cpp_helper/cpphelper_epoll.hpp"
#include "os_tools_net.h"
#include "os_tools_net_socket.hpp"
#include "sntp_client.h"

#define USING_EPOLL 1

static ssize_t os_tools_sntp_client_send(int fd)
{
    //
    auto send_pkt = ntp_min_packet_gen_for_client();
    //
    uint8_t send_buf[sizeof(NtpMinPacket)];
    //
    int send_bufsz = ntp_min_packet_to_client_netdata(send_pkt, send_buf, sizeof(send_buf));
    //
    auto wc = CppSocket::write_data(fd, send_buf, send_bufsz);
    if (wc < 0)
    {
        //
        OS_LOGD("errno:%s\n", errno, strerror(errno));
        return wc;
    }
    if (wc != send_bufsz)
    {
        OS_LOGV("buf(sent:%zd/%d):\n%s", wc, send_bufsz, OS_LOG_HEXDUMP(send_buf, send_bufsz));
        return -1;
    }
    return 0;
}

static ssize_t os_tools_sntp_client_recv(int fd, uint64_t* output_epoch_time, uint64_t reference_epoch_time = 0)
{
    uint8_t recv_buf[sizeof(NtpMinPacket)];
    size_t  recv_bufsz = sizeof(recv_buf);
    //
    auto rc = CppSocket::read_data(fd, (uint8_t*)recv_buf, recv_bufsz);
    if (rc < 0)
    {
        //
        OS_LOGD("errno:%s\n", errno, strerror(errno));
        return rc;
    }
    if (rc < (int)recv_bufsz)
    {
        OS_LOGV("buf(recv:%zd/%zu):\n%s", rc, recv_bufsz, OS_LOG_HEXDUMP(recv_buf, recv_bufsz));
        return -1;
    }
    auto recv_pkt = ntp_min_packet_from_server_netdata(recv_buf, recv_bufsz);
    if (0)
    {
        //
        OS_LOGV("recv_pkt(len=%zu):\n%s", sizeof(recv_pkt), OS_LOG_HEXDUMP(&recv_pkt, sizeof(recv_pkt)));
    }
    //
    auto timelapse_ms = ntp_min_packet_from_server_is_good(recv_pkt);
    //
    if (timelapse_ms)
    {
        auto unix_ts = ntp_ts_to_unix_time(recv_pkt.TransmitTimestamp, reference_epoch_time);
        if (0)
        {
#if defined(SNTP_CLIENT_CHECK_TIMELAPSE) && SNTP_CLIENT_CHECK_TIMELAPSE != 0
            //
            OS_LOGV("timelapse_ms=%d", timelapse_ms);
#endif
            OS_LOGV("recv_epoch_time=%" PRIu64 ", sys_epoch_time=%" PRIu64 "", unix_ts, (uint64_t)os_get_epoch_time());
        }
        //
        if (output_epoch_time)
        {
            *output_epoch_time = unix_ts;
        }
    }
    return 0;
}

//
int os_tools_sntp_get_epoch_impl(CppSocket& sock, time_t* ptime)
{
    int ret = -1, *pret = &ret;

    //
    EpollHelper::fd_set_block(sock.get(), 0);

    //
    ret = -1;
    EpollHelper::fd_timeout_write(pret,
                                  sock.get(),
                                  SNTP_CLIENT_MAX_TIMELAPSE_MS,
                                  [&](int fd, int event, void* pret)
                                  {
                                      //
                                      *(int*)pret = (int)os_tools_sntp_client_send(fd);
                                  });
    if (ret < 0)
    {
        return ret;
    }

    //
    ret = -1;
    EpollHelper::fd_timeout_read(pret,
                                 sock.get(),
                                 SNTP_CLIENT_MAX_TIMELAPSE_MS,
                                 [&](int fd, int event, void* pret)
                                 {
                                     uint64_t out_time;
                                     uint64_t ref_time = time(NULL);
                                     //
                                     *(int*)pret = (int)os_tools_sntp_client_recv(fd, &out_time, ref_time);
                                     //
                                     if (ptime)
                                     {
                                         *ptime = out_time;
                                     }
                                 });
    if (ret < 0)
    {
        return ret;
    }

    return 0;
}


int os_tools_sntp_get_epoch_by_ip4(struct in_addr ipv4_addr, uint16_t port, time_t* ptime)
{
    int sa_family = AF_INET;
    //
    CppSocket sock(sa_family, SOCK_DGRAM, 0);
    if (!sock.isValid())
    {
        return -1;
    }
    if (sock.connect_ipv4(ipv4_addr, port))
    {
        return -1;
    }
    return os_tools_sntp_get_epoch_impl(sock, ptime);
}


int os_tools_sntp_get_epoch_by_ip4_str(const char* ipv4_str, uint16_t port, time_t* ptime)
{
    int sa_family = AF_INET;
    //
    CppSocket sock(sa_family, SOCK_DGRAM, 0);
    if (!sock.isValid())
    {
        return -1;
    }
    if (sock.connect_ipv4(ipv4_str, port))
    {
        return -1;
    }
    return os_tools_sntp_get_epoch_impl(sock, ptime);
}


int os_tools_sntp_get_epoch_by_ip6(struct in6_addr ipv6_addr, uint16_t port, time_t* ptime)
{
    int sa_family = AF_INET6;
    //
    CppSocket sock(sa_family, SOCK_DGRAM, 0);
    if (!sock.isValid())
    {
        return -1;
    }
    if (sock.connect_ipv6(ipv6_addr, port))
    {
        return -1;
    }
    return os_tools_sntp_get_epoch_impl(sock, ptime);
}


int os_tools_sntp_get_epoch_by_ip6_str(const char* ipv6_str, uint16_t port, time_t* ptime)
{
    int sa_family = AF_INET6;
    //
    CppSocket sock(sa_family, SOCK_DGRAM, 0);
    if (!sock.isValid())
    {
        return -1;
    }
    if (sock.connect_ipv6(ipv6_str, port))
    {
        return -1;
    }
    return os_tools_sntp_get_epoch_impl(sock, ptime);
}


int os_tools_sntp_get_epoch_by_server(const char* server_domain_name, uint16_t port, time_t* ptime)
{
    int ret = -1;
    //
    if (port == 0)
    {
        port = 123;
    }
    if (!server_domain_name)
    {
        server_domain_name = "pool.ntp.org";
    }
    std::list<struct in_addr>  ipv4_addrs;
    std::list<struct in6_addr> ipv6_addrs;
    os_net_nslookup(server_domain_name, ipv4_addrs, ipv6_addrs);
    //
    for (auto& ip4 : ipv4_addrs)
    {
        ret = os_tools_sntp_get_epoch_by_ip4(ip4, port, ptime);
        if (ret == 0)
        {
            return ret;
        }
    }
    //
    for (auto& ip6 : ipv6_addrs)
    {
        ret = os_tools_sntp_get_epoch_by_ip6(ip6, port, ptime);
        if (ret == 0)
        {
            return ret;
        }
    }
    return ret;
}
