#ifndef OS_TOOLS_TIME_H
#define OS_TOOLS_TIME_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif


int32_t os_set_epoch_time(time_t seconds);

time_t os_get_epoch_time();

uint64_t os_get_timestamp_ms();

uint64_t os_get_timelapse_ms(uint64_t cmpTs);

char *os_logts_str(char *buffer, size_t buffer_len);

uint64_t os_logts_ms();


#ifdef __cplusplus
}
#endif


#endif /* OS_TOOLS_TIME_H */
