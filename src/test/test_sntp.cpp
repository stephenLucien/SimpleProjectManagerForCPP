#include <inttypes.h>
#include <unistd.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
//
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_net_sntp.h"

static int test_sntp(int reason, void* userdata)
{
    int ret = -1;
    //
    time_t epoch_time;
    //
    ret = os_tools_sntp_get_epoch(&epoch_time);
    //
    if (ret != 0)
    {
        return ret;
    }
    OS_LOGI("recv_epoch_time=%" PRIu64 ", sys_epoch_time=%" PRIu64 "", (uint64_t)epoch_time, (uint64_t)os_get_epoch_time());

    return 0;
}

REG_TEST_FUNC(test_sntp, test_sntp, NULL)
