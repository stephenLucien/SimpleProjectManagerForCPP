#ifndef __OS_TOOLS_NET_SNTP_H__
#define __OS_TOOLS_NET_SNTP_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int os_tools_sntp_get_epoch(const char* ipv4_str, uint16_t port, time_t* ptime);

#ifdef __cplusplus
}
#endif


#endif  // __OS_TOOLS_NET_SNTP_H__
