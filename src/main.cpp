#include <cstring>
#include <iostream>

#include "app/app.h"
#include "utils/compile_time.h"
#include "utils/curl_wrapper.hpp"
#include "utils/os_tools.h"

int main(int argc, char *argv[])
{
    os_setup_backtrace();
    //
    OS_PRINT(
        "%s:\n"
        "\tThis app built at %s",
        argv[0],
        COMPILE_TIME_STR(app));
    //
    OS_PRINT("hello world");
    //
    std::string url = "www.baidu.com";
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
    auto ret = curl_get_basic(url, header, query, buffer, timeout_conn_ms, timeout_ms, en_debug);
    //
    OS_PRINT("data:\n%s", (char *)buffer.data());

    return 0;
}
