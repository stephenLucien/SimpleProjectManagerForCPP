#include "os_tools_time.h"

//
#include "os_tools_log.h"

//
#include <sys/time.h>
#include <cerrno>
#include <cstdint>
#include <cstring>


int32_t os_set_epoch_time(time_t seconds)
{
    int ret = -1;

    //
#if 1
    struct timeval tv = {.tv_sec = seconds, .tv_usec = 0};
    //
    ret = settimeofday(&tv, NULL);
#else
    ret = stime(&seconds);
#endif

    if (ret)
    {
        os_log_printf(OS_LOG_ERR, "os", "settimeofday failed, ret=%d, %s", ret, strerror(errno));
    } else
    {
        os_log_printf(OS_LOG_INFO, "os", "settimeofday okay");
    }
    return ret;
}

time_t os_get_epoch_time()
{
    time_t curTime;
    time(&curTime);
    return curTime;
}

uint64_t os_get_timestamp_ms()
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    uint64_t ts;
    ts = (uint64_t)monotime.tv_sec * 1000 + monotime.tv_nsec / 1000 / 1000;
    return ts;
}

uint64_t os_get_timelapse_ms(uint64_t cmpTs)
{
    auto curts = os_get_timestamp_ms();
    //
    auto lapse_ms = curts;
    //
    if (curts >= cmpTs)
    {
        lapse_ms = curts - cmpTs;
    } else
    {
        // overflow: UINT64_MAX + 1 + curts - cmpTs = UINT64_MAX - ( cmpTs - 1 - curts )
        lapse_ms = UINT64_MAX - (cmpTs - 1 - curts);
    }
    // OS_LOGD("curts=%" PRIu64 ", cmpTs=%" PRIu64 ", lapse_ms=%" PRIu64 "", curts, cmpTs, lapse_ms);
    return lapse_ms;
}


char *os_logts_str(char *buffer, size_t buffer_len)
{
    time_t curTime;
    time(&curTime);

    struct tm curTm;
    localtime_r(&curTime, &curTm);
    strftime(buffer, buffer_len, "%Y%m%d_%H%M%S", &curTm);
    return buffer;
}

uint64_t os_logts_ms()
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    uint64_t ts;
    ts = (uint64_t)monotime.tv_sec * 1000 + monotime.tv_nsec / 1000 / 1000;
    return ts;
}
