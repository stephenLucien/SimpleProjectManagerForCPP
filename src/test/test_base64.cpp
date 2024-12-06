#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/base64_codec.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"

static const char* data0 = "hello";
static const char* data1 = "hell";
static const char* data2 = "hel";

static int test_base64(int reason, void* userdata)
{
    size_t ol0, ol1, ol2;
    //
    ol0 = strlen(data0);
    ol1 = strlen(data1);
    ol2 = strlen(data2);
    //
    auto b64_0 = base64_encode(NULL, 0, (uint8_t*)data0, ol0);
    auto b64_1 = base64_encode(NULL, 0, (uint8_t*)data1, ol1);
    auto b64_2 = base64_encode(NULL, 0, (uint8_t*)data2, ol2);
    //
    size_t l0, l1, l2;
    //
    auto d0 = base64_decode(NULL, 0, b64_0, 0, &l0);
    auto d1 = base64_decode(NULL, 0, b64_1, 0, &l1);
    auto d2 = base64_decode(NULL, 0, b64_2, 0, &l2);
    //
    OS_LOGD(
        "\n"
        "ori(len=%zu):%s\n"
        "b64:%s\n"
        "res(len=%zu):%s\n",
        ol0,
        data0,
        b64_0,
        l0,
        d0);
    //
    OS_LOGD(
        "\n"
        "ori(len=%zu):%s\n"
        "b64:%s\n"
        "res(len=%zu):%s\n",
        ol1,
        data1,
        b64_1,
        l1,
        d1);
    //
    OS_LOGD(
        "\n"
        "ori(len=%zu):%s\n"
        "b64:%s\n"
        "res(len=%zu):%s\n",
        ol2,
        data2,
        b64_2,
        l2,
        d2);
    //
    if (b64_0)
    {
        free((void*)b64_0);
    }
    if (b64_1)
    {
        free((void*)b64_1);
    }
    if (b64_2)
    {
        free((void*)b64_2);
    }
    if (d0)
    {
        free((void*)d0);
    }
    if (d1)
    {
        free((void*)d1);
    }
    if (d2)
    {
        free((void*)d2);
    }
    return 0;
}

REG_TEST_FUNC(test_base64, test_base64, NULL)
