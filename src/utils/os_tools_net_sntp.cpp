#include "os_tools_net_sntp.h"

//
#include "cpp_helper/cpphelper_epoll.hpp"
#include "os_tools_net_socket.hpp"
#include "utils/sntp_client.h"


int os_tools_sntp_get_epoch(const char* ipv4_str, uint16_t port, time_t* ptime)
{
    int ret = -1;
    //
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
#define USING_EPOLL 1

#if defined(USING_EPOLL) && USING_EPOLL != 0
    //
    EpollHelper ep;
    //
    EpollHelper::fd_set_block(sock.get(), 0);
#endif
    //
    auto send_pkt = ntp_min_packet_gen_for_client();
    //
    uint8_t send_buf[sizeof(send_pkt)];
    //
    size_t send_bufsz = ntp_min_packet_to_client_netdata(send_pkt, send_buf, sizeof(send_buf));
    //
    OS_LOGV("send_pkt(len=%zu):\n%s", send_bufsz, OS_LOG_HEXDUMP(&send_pkt, send_bufsz));
    OS_LOGV("sendbuf(len=%zu):\n%s", send_bufsz, OS_LOG_HEXDUMP(send_buf, send_bufsz));
    //
    auto wfunc = [&](int fd, int event, void* pret)
    {
        //
        auto wc = CppSocket::write_data(fd, (uint8_t*)send_buf, send_bufsz);
        if (wc < 0)
        {
            //
            OS_LOGD("errno:%s\n", errno, strerror(errno));
        }
        if (wc > 0)
        {
            OS_LOGV("buf(len=%zu):\n%s", wc, OS_LOG_HEXDUMP(send_buf, wc));
        }
        //
        *(int*)pret = (int)wc;
    };
    //
    ret = -1;

#if defined(USING_EPOLL) && USING_EPOLL != 0
    //
    ep.fd_add(sock.get(), EPOLLOUT, wfunc, &ret);
    //
    if (ep.check(1000) <= 0)
    {
        OS_LOGD("timeout or error");
    }
    if (ret != send_bufsz)
    {
        OS_LOGD("ret=%d", ret);
        return -1;
    }
#else
    wfunc(sock.get(), 0, &ret);
#endif
    OS_LOGD("wc=%d", ret);

    //
    uint8_t recv_buf[sizeof(send_pkt)];
    size_t  recv_bufsz = sizeof(recv_buf);
    //
    auto rfunc = [&](int fd, int event, void* pret)
    {
        //
        auto rc = CppSocket::read_data(fd, (uint8_t*)recv_buf, recv_bufsz);
        if (rc < 0)
        {
            //
            OS_LOGD("errno:%s\n", errno, strerror(errno));
        }
        if (rc > 0)
        {
            OS_LOGV("buf(len=%zu):\n%s", rc, OS_LOG_HEXDUMP(recv_buf, rc));
        }
        //
        *(int*)pret = (int)rc;
    };
    //
    ret = -1;

#if defined(USING_EPOLL) && USING_EPOLL != 0
    //
    ep.fd_add(sock.get(), EPOLLIN, rfunc, &ret);
    //
    if (ep.check(5000) <= 0)
    {
        OS_LOGD("timeout or error");
    }
    if (ret != recv_bufsz)
    {
        OS_LOGD("ret=%d", ret);
        return -1;
    }
#else
    rfunc(sock.get(), 0, &ret);
#endif
    OS_LOGD("rc=%d", ret);


    auto recv_pkt = ntp_min_packet_from_server_netdata(recv_buf, recv_bufsz);
    //
    OS_LOGV("recvbuf(len=%zu):\n%s", recv_bufsz, OS_LOG_HEXDUMP(recv_buf, recv_bufsz));
    OS_LOGV("recv_pkt(len=%zu):\n%s", recv_bufsz, OS_LOG_HEXDUMP(&recv_pkt, recv_bufsz));
    auto timelapse_ms = ntp_min_packet_from_server_is_good(recv_pkt);
    if (timelapse_ms)
    {
        //
        OS_LOGI("timelapse_ms=%d, recv_epoch_time=%u, sys_epoch_time=%lu",
                timelapse_ms,
                ntp_ts_to_unix_time(recv_pkt.TransmitTimestamp),
                os_get_epoch_time());
    }

    if (ptime)
    {
        *ptime = ntp_ts_to_unix_time(recv_pkt.TransmitTimestamp);
    }

    return 0;
}
