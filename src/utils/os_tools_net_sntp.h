#ifndef __OS_TOOLS_NET_SNTP_H__
#define __OS_TOOLS_NET_SNTP_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int os_tools_sntp_get_epoch_by_ip4(struct in_addr ipv4_addr, uint16_t port, time_t* ptime);

int os_tools_sntp_get_epoch_by_ip4_str(const char* ipv4_str, uint16_t port, time_t* ptime);

int os_tools_sntp_get_epoch_by_ip6(struct in6_addr ipv6_addr, uint16_t port, time_t* ptime);

int os_tools_sntp_get_epoch_by_ip6_str(const char* ipv6_str, uint16_t port, time_t* ptime);

int os_tools_sntp_get_epoch_by_server(const char* server_domain_name, uint16_t port, time_t* ptime);

static inline int os_tools_sntp_get_epoch(time_t* ptime)
{
    return os_tools_sntp_get_epoch_by_server(NULL, 0, ptime);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
    #include "cpp_helper/cpphelper_socket.hpp"
int os_tools_sntp_get_epoch_impl(CppSocket& sock, time_t* ptime);

#endif


#endif  // __OS_TOOLS_NET_SNTP_H__
