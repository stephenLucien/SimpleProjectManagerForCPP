#include "cpp_helper/cpphelper_curl.hpp"

//
#include <string>
#include <unordered_map>
#include <vector>
#include "nlohmann/json.hpp"
#include "utils/os_tools.h"



int curl_download_file(const char* url, const char* outputfile, int timeout_conn_ms, int timeout_ms, int en_debug)
{
    int ret = -1;
    if (!url || !outputfile)
    {
        return ret;
    }

    CurlWrapper curl_cpp(en_debug);
    if (!curl_cpp.isValid())
    {
        os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
        return ret;
    }
    ret = curl_cpp.setupWriteFile(outputfile);
    if (ret)
    {
        return ret;
    }
    curl_cpp.setUrl(url);
    curl_cpp.setupTimeout(timeout_ms, timeout_conn_ms);

    ret = curl_cpp.perform();

    return ret;
}


int curl_delete_basic(const std::string&                                  url,
                      const std::unordered_map<std::string, std::string>& header,
                      const std::unordered_map<std::string, std::string>& body,
                      std::vector<uint8_t>&                               buffer,
                      int                                                 timeout_conn_ms,
                      int                                                 timeout_ms,
                      int                                                 en_debug)
{
    int ret = -1;

    //
    CurlWrapper curl_cpp(en_debug);
    /* get a curl handle */
    if (!curl_cpp.isValid())
    {
        os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
        return ret;
    }

    curl_cpp.setUrl(url);
    curl_cpp.setupHeaders(header);
    std::string bodies = CurlWrapper::genFormDataString(body, false);
    if (bodies.length() > 0)
    {
        curl_cpp.setPOSTFIELDS(bodies.c_str(), bodies.length());
    }
    curl_cpp.setupTimeout(timeout_ms, timeout_conn_ms);
    //
    if (buffer.size() == 0)
    {
        // allocate buffer if not
        buffer.resize(4096);
    }
    curl_cpp.setupWriteBuffer(buffer.data(), buffer.size());
    //
    curl_cpp.setMethod_DELETE();
    //
    ret = curl_cpp.perform();

    return ret;
}


int curl_get_basic(std::string                                         url,
                   const std::unordered_map<std::string, std::string>& header,
                   const std::unordered_map<std::string, std::string>& query,
                   std::vector<uint8_t>&                               buffer,
                   int                                                 timeout_conn_ms,
                   int                                                 timeout_ms,
                   int                                                 en_debug)
{
    int ret = -1;

    //
    CurlWrapper curl_cpp(en_debug);
    /* get a curl handle */
    if (!curl_cpp.isValid())
    {
        os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
        return ret;
    }

    curl_cpp.setUrl(url, query);
    curl_cpp.setupHeaders(header);
    curl_cpp.setupTimeout(timeout_ms, timeout_conn_ms);
    //
    if (buffer.size() == 0)
    {
        // allocate buffer if not
        buffer.resize(4096);
    }
    curl_cpp.setupWriteBuffer(buffer.data(), buffer.size());
    //
    curl_cpp.setMethod_GET();
    //
    ret = curl_cpp.perform();

    return ret;
}


int curl_post_basic(const std::string&                                  url,
                    const std::unordered_map<std::string, std::string>& header,
                    const std::string&                                  bodies,
                    std::vector<uint8_t>&                               buffer,
                    int                                                 timeout_conn_ms,
                    int                                                 timeout_ms,
                    int                                                 en_debug)
{
    int ret = -1;

    //
    CurlWrapper curl_cpp(en_debug);
    /* get a curl handle */
    if (!curl_cpp.isValid())
    {
        os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
        return ret;
    }

    curl_cpp.setUrl(url);
    curl_cpp.setupHeaders(header);
    if (bodies.length() > 0)
    {
        curl_cpp.setPOSTFIELDS(bodies.c_str(), bodies.length());
    }
    curl_cpp.setupTimeout(timeout_ms, timeout_conn_ms);
    //
    if (buffer.size() == 0)
    {
        // allocate buffer if not
        buffer.resize(4096);
    }
    curl_cpp.setupWriteBuffer(buffer.data(), buffer.size());
    //
    curl_cpp.setMethod_POST();
    //
    ret = curl_cpp.perform();

    return ret;
}



int curl_post_form_basic(const std::string&                                  url,
                         const std::unordered_map<std::string, std::string>& in_header,
                         const std::unordered_map<std::string, std::string>& body,
                         std::vector<uint8_t>&                               buffer,
                         int                                                 timeout_conn_ms,
                         int                                                 timeout_ms,
                         int                                                 en_debug)
{
    int ret = -1;
    //
    auto header = in_header;
    //
    if (header.find("Content-Type") == header.end())
    {
        header["Content-Type"] = "application/x-www-form-urlencoded;charset=utf-8";
    }

    std::string bodies = CurlWrapper::genFormDataString(body, false);

    ret = curl_post_basic(url, header, bodies, buffer, timeout_conn_ms, timeout_ms, en_debug);

    return ret;
}


int curl_post_json_basic(const std::string&                                  url,
                         const std::unordered_map<std::string, std::string>& in_header,
                         const nlohmann::json&                               body,
                         std::vector<uint8_t>&                               buffer,
                         int                                                 timeout_conn_ms,
                         int                                                 timeout_ms,
                         int                                                 en_debug)
{
    int ret = -1;
    //
    auto header = in_header;
    //
    if (header.find("Content-Type") == header.end())
    {
        header["Content-Type"] = "application/json;charset=utf-8";
    }

    std::string bodies;
    if (!body.empty())
    {
        bodies = body.dump();
    }

    ret = curl_post_basic(url, header, bodies, buffer, timeout_conn_ms, timeout_ms, en_debug);

    return ret;
}
