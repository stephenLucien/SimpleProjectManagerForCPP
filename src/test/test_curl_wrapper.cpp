

#include "utils/curl_wrapper.hpp"
#include "utils/os_tools.h"

//
#include "manager/test_manager.h"

int test_curl_wrapper(int reason, void* userdata)
{
    const char* in = (const char*)userdata;
    //
    std::string url = in ? in : "www.baidu.com";
    //
    std::unordered_map<std::string, std::string> header;
    //
    std::unordered_map<std::string, std::string> query;
    //
    std::vector<uint8_t> buffer(1024 * 128);
    //
    int timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS;
    int timeout_ms      = DEFAULT_CURL_TIMEOUT_MS;
    int en_debug        = 1;
    //
    memset(buffer.data(), 0, buffer.size());
    //
    auto ret = curl_get_basic(url, header, query, buffer, timeout_conn_ms, timeout_ms, en_debug);
    //
    OS_PRINT("data(len=%zu):\n%s", strlen((char*)buffer.data()), (char*)buffer.data());
    //
    return ret;
}


REG_TEST_FUNC(curl_wrapper_get_baidu, test_curl_wrapper, "www.baidu.com")
REG_TEST_FUNC(curl_wrapper_get_google, test_curl_wrapper, "www.google.com")
