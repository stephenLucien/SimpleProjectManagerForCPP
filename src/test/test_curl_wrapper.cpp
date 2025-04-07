

#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

//
#include "cpp_helper/cpphelper_curl.hpp"
#include "manager/test_manager.h"
#include "utils/os_tools_system.h"

#include "manager/cmdline_argument_manager.h"



int test_curl_wrapper(int reason, void *userdata)
{
    const char *in = (const char *)userdata;
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
    OS_PRINT("data(len=%zu):\n%s", strlen((char *)buffer.data()), (char *)buffer.data());
    //
    return ret;
}


REG_TEST_FUNC(curl_wrapper_get_baidu, test_curl_wrapper, "www.baidu.com")
REG_TEST_FUNC(curl_wrapper_get_google, test_curl_wrapper, "www.google.com")

//
static std::string m_url = "www.baidu.com";
//
REG_CMDLINE_ARG_PARSE_FUNC(url, 'U', "url", 1, {
    //
    if (_param)
    {
        m_url = _param;
    }
})
//
static std::string m_file = "/tmp/baidu.html";
//
REG_CMDLINE_ARG_PARSE_FUNC(path, 'F', "file", 1, {
    //
    if (_param)
    {
        m_file = _param;
    }
})
//
int test_curl_downloader(int reason, void *userdata)
{
    OS_LOGV(
        "download: %s\n"
        "tofile:%s",
        m_url.c_str(),
        m_file.c_str());
    CurlFileDownloader downloader(m_url, m_file);
    //
    downloader.getWriter()->setPrint(true);
    downloader.run("FileDownloader");
    while (m_app_running && downloader.isRunning())
    {
        sleep(1);
    }
    if (downloader.isRunning())
    {
        OS_LOGI("");
        downloader.requestExitAndWait();
        OS_LOGI("");
    }
    if (downloader.isDownloaded())
    {
        OS_LOGI("Downloaded! file:%s, url:%s", m_file.c_str(), m_url.c_str());
    } else
    {
        OS_LOGI("Download failed! file:%s, url:%s", m_file.c_str(), m_url.c_str());
    }

    return 0;
}

REG_TEST_FUNC(test_curl_downloader, test_curl_downloader, NULL)
REG_TEST_FUNC(download, test_curl_downloader, NULL)
