#include "os_tools_log.h"
#include "os_tools_net.h"
#include "cpp_helper/cpphelper_socket.hpp"


//
#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>


#define BUFSIZE 8192

struct route_info
{
    //
    struct in_addr dstAddr;
    //
    struct in_addr srcAddr;
    //
    struct in_addr gateWay;
    //
    char ifName[IF_NAMESIZE];
};


static int readNlSock(int sockFd, char *bufPtr, size_t buf_size, int seqNum, int pId)
{
    int ret = -1;
    //
    struct nlmsghdr *nlHdr;
    //
    int readLen = 0, msgLen = 0;

    do
    {
        /* Recieve response from the kernel */
        readLen = recv(sockFd, bufPtr, buf_size - msgLen, 0);
        if (readLen < 0)
        {
            OS_LOGE("sockFd=%d, recv err: %s", sockFd, strerror(errno));
            return ret;
        }

        nlHdr = (struct nlmsghdr *)bufPtr;

        /* Check if the header is valid */
        if ((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
        {
            OS_LOGE("Error in recieved packet");
            return ret;
        }

        /* Check if the its the last message */
        if (nlHdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        } else
        {
            /* Else move the pointer to buffer appropriately */
            bufPtr += readLen;
            msgLen += readLen;
        }

        /* Check if its a multi part message */
        if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            /* return if its not */
            break;
        }
    } while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

    return msgLen;
}

/* parse the route info returned */
static int parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)
{
    int ret = -1;
    //
    struct rtmsg *rtMsg;
    //
    struct rtattr *rtAttr;
    //
    int rtLen;
    //
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    /* If the route is not for AF_INET or does not belong to main routing table then return. */
    if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
    {
        return ret;
    }

    /* get the rtattr field */
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    rtLen  = RTM_PAYLOAD(nlHdr);

    for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen))
    {
        switch (rtAttr->rta_type)
        {
            case RTA_OIF:
                if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
                break;

            case RTA_GATEWAY:
                memcpy(&rtInfo->gateWay, RTA_DATA(rtAttr), sizeof(rtInfo->gateWay));
                break;

            case RTA_PREFSRC:
                memcpy(&rtInfo->srcAddr, RTA_DATA(rtAttr), sizeof(rtInfo->srcAddr));
                break;

            case RTA_DST:
                memcpy(&rtInfo->dstAddr, RTA_DATA(rtAttr), sizeof(rtInfo->dstAddr));
                break;
        }
    }

    return 0;
}


// meat
int os_net_iface_get_ipv4_gwaddr(const char *iface, struct in_addr *p_ip4, int recv_timeout_ms, int send_timeout_ms)
{
    int ret = -1;

    /* Create Socket */
    CppSocket sockObj(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (!sockObj.isValid())
    {
        OS_LOGE("");
        return ret;
    }
    //
    if (iface)
    {
        ret = sockObj.bind_to_device(iface);
        if (ret < 0)
        {
            OS_LOGE("");
            return ret;
        }
    }
    if (recv_timeout_ms > 0)
    {
        sockObj.set_recv_timeout(recv_timeout_ms / 1000, (recv_timeout_ms % 1000) * 1000);
    }
    if (send_timeout_ms > 0)
    {
        sockObj.set_send_timeout(send_timeout_ms / 1000, (send_timeout_ms % 1000) * 1000);
    }

    //
    auto sock = sockObj.get();

    //
    int found_gatewayip = 0;
    //
    struct nlmsghdr *nlMsg;
    //
    struct rtmsg *rtMsg;
    //
    struct route_info route_info;
    // pretty large buffer
    char msgBuf[BUFSIZE];
    //
    int len, msgSeq = 0;

    /* Initialize the buffer */
    memset(msgBuf, 0, sizeof(msgBuf));

    /* point the header and the msg structure pointers into the buffer */
    nlMsg = (struct nlmsghdr *)msgBuf;
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);

    /* Fill in the nlmsg header*/
    nlMsg->nlmsg_len  = NLMSG_LENGTH(sizeof(struct rtmsg));  // Length of message.
    nlMsg->nlmsg_type = RTM_GETROUTE;                        // Get the routes from kernel routing table .

    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;  // The message is a request for dump.
    nlMsg->nlmsg_seq   = msgSeq++;                    // Sequence of the message packet.
    nlMsg->nlmsg_pid   = getpid();                    // PID of process sending the request.

    /* Send the request */
    if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
    {
        OS_LOGE("sockFd=%d, send err: %s", sock, strerror(errno));
        return -1;
    }

    /* Read the response */
    len = readNlSock(sock, msgBuf, sizeof(msgBuf), msgSeq, getpid());
    if ((len) < 0)
    {
        OS_LOGE("Read From Socket Failed...");
        return -1;
    }

    /* Parse and print the response */
    for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len))
    {
        memset(&route_info, 0, sizeof(route_info));
        if (parseRoutes(nlMsg, &route_info) < 0)
        {
            continue;  // don't check route_info if it has not been set up
        }
        // OS_LOGV("dstAddr=%s", IPV4_ADDR2STR(&route_info.dstAddr));

        // Check if default gateway
        if (route_info.dstAddr.s_addr == 0)
        {
            if (p_ip4)
            {
                *p_ip4 = route_info.gateWay;
            } else
            {
                OS_LOGD("iface(%s),gw=%s", iface ? iface : "", IPV4_ADDR2STR(&route_info.gateWay));
            }
            found_gatewayip = 1;
            break;
        }
    }

    return found_gatewayip;
}


// meat
char *os_net_iface_get_ipv4_gwaddr_str(const char *iface, char *buf, size_t bufsz)
{
    if (!buf || bufsz <= 0)
    {
        return buf;
    }
    memset(buf, 0, bufsz);

    struct in_addr ip4, *p_ip4 = &ip4;
    //
    if (os_net_iface_get_ipv4_gwaddr(iface, p_ip4, 500, 500) > 0)
    {
        ipv4_addr2str(p_ip4, buf, bufsz);
    }

    return buf;
}
