#include "cpp_helper/cpphelper_curl.hpp"

//
#include <string>
#include <unordered_map>
#include <vector>
#include "nlohmann/json.hpp"
#include "utils/os_tools.h"


typedef struct
{
    FILE*   fp;
    size_t* wc;
} CurlWriteData;

static size_t curl_fwrite(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto data = (CurlWriteData*)userdata;
    if (!data)
    {
        return 0;
    }
    size_t written = fwrite(ptr, size, nmemb, data->fp);
    if (data->wc)
    {
        *(data->wc) += written;
    }
    return written;
}



int curl_download_file(const char* url, const char* outputfile, size_t* wc, int timeout_conn_ms, int timeout_ms, int en_debug)
{
    int ret = -1;
    if (wc)
    {
        *wc = 0;
    }
    if (!url || !outputfile)
    {
        return ret;
    }


    CURL_WRAPPER(curl, en_debug);
    FILE*    fp = NULL;
    CURLcode res;


    do
    {
        fp = fopen(outputfile, "wb");
        if (!fp)
        {
            os_log_printf(OS_LOG_ERR, "curl", "fail open %s", outputfile);
            break;
        }

        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            break;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        CurlWriteData data;
        data.fp = fp;
        data.wc = wc;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        //
        if (timeout_conn_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_conn_ms);
        }
        if (timeout_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_perform() failed: %d,%s", res, curl_easy_strerror(res));
            break;
        }
        ret = 0;
    } while (0);

    if (fp)
    {
        fclose(fp);
    }

    return ret;
}


int curl_get_file_size(const char* url, size_t* sz, int timeout_conn_ms, int timeout_ms, int en_debug)
{
    int ret = -1;
    if (sz)
    {
        *sz = 0;
    }
    if (!url)
    {
        return ret;
    }

    CURL_WRAPPER(curl, en_debug);
    CURLcode res;


    do
    {
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            break;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        //
        if (timeout_conn_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_conn_ms);
        }
        if (timeout_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_perform() failed: %d,%s", res, curl_easy_strerror(res));
            break;
        }
        double length_sz = 0;
        //
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length_sz);

        if (res != CURLE_OK)
        {
            os_log_printf(OS_LOG_ERR,
                          "curl",
                          "curl_easy_getinfo(CURLINFO_CONTENT_LENGTH_DOWNLOAD) "
                          "failed: %s",
                          curl_easy_strerror(res));
            break;
        }
        ret = 0;
        if (sz)
        {
            *sz = length_sz;
        }
        if (1)
        {
            os_log_printf(OS_LOG_DEBUG, "curl", "url:%s, len=%d ", url, *sz);
        }
    } while (0);

    return ret;
}


typedef struct
{
    uint8_t* buf;
    size_t   bufsz;
    size_t   wc;
} CurlHttpWriteData;

static size_t http_write_cb(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    int len = 0;
    if (!userdata)
    {
        return len;
    }
    auto data = (CurlHttpWriteData*)userdata;
    //
    len = size * nmemb;
    if (!(data->wc + len <= data->bufsz))
    {
        os_log_printf(OS_LOG_ERR, "curl", "buf small");
        return len;
    }
    auto dst = data->buf + data->wc;
    memcpy(dst, ptr, len);
    data->wc += len;

    return len;
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

    struct curl_slist* headers = NULL;
    //
    if (buffer.size() == 0)
    {
        // allocate buffer if not
        buffer.resize(4096);
    }
    //
    CURL_WRAPPER(curl, en_debug);
    CURLcode res;

    CurlHttpWriteData userdata;
    userdata.buf   = buffer.data();
    userdata.bufsz = buffer.size();
    userdata.wc    = 0;
    memset(userdata.buf, 0, userdata.bufsz);

    for (auto& e : header)
    {
        auto obj = e.first + ": " + e.second;
        if (!headers)
        {
            headers = curl_slist_append(NULL, obj.c_str());
        } else
        {
            curl_slist_append(headers, obj.c_str());
        }
    }

    std::string bodies;
    do
    {
        for (auto& e : body)
        {
            bodies += e.first + "=" + e.second + "&";
        }
        if (bodies.length() > 0)
        {
            if (bodies.at(bodies.length() - 1) == '&')
            {
                bodies.at(bodies.length() - 1) = '\0';
            }
        }
    } while (0);
    do
    {
        /* get a curl handle */
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            break;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        //
        if (timeout_conn_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_conn_ms);
        }
        if (timeout_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        }

        /* Now specify the POST data */

        if (headers)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        if (bodies.length() > 0)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodies.c_str());
            if (en_debug)
            {
                os_log_printf(OS_LOG_DEBUG, "curl", "(len=%d) %s", bodies.length(), bodies.c_str());
            }
        }



        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userdata);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);


        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_perform() failed: %d,%s", res, curl_easy_strerror(res));
            break;
        }

        ret = 0;
    } while (0);

    if (headers)
    {
        curl_slist_free_all(headers);
    }

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

    std::string bodies;

    struct curl_slist* headers = NULL;
    //
    if (buffer.size() == 0)
    {
        // allocate buffer if not
        buffer.resize(4096);
    }
    //
    CURL_WRAPPER(curl, en_debug);
    CURLcode res;

    CurlHttpWriteData userdata;
    userdata.buf   = buffer.data();
    userdata.bufsz = buffer.size();
    userdata.wc    = 0;
    memset(userdata.buf, 0, userdata.bufsz);

    for (auto& e : header)
    {
        auto obj = e.first + ": " + e.second;
        if (!headers)
        {
            headers = curl_slist_append(NULL, obj.c_str());
        } else
        {
            curl_slist_append(headers, obj.c_str());
        }
    }

    std::string queries;
    do
    {
        for (auto& e : query)
        {
            auto tmpkey  = e.first;
            auto tmpval  = e.second;
            auto ptr_key = curl_escape(tmpkey.c_str(), tmpkey.length());
            auto ptr_val = curl_escape(tmpval.c_str(), tmpval.length());

            queries += (ptr_key ? std::string(ptr_key) : e.first) + "=" + (ptr_val ? std::string(ptr_val) : e.second) + "&";

            if (ptr_key)
            {
                curl_free(ptr_key);
            }
            if (ptr_val)
            {
                curl_free(ptr_val);
            }
        }
        if (queries.length() > 0)
        {
            if (queries.at(queries.length() - 1) == '&')
            {
                queries.at(queries.length() - 1) = '\0';
            }
        }
    } while (0);
    if (!queries.empty())
    {
        url += "?" + queries;
    }

    do
    {
        /* get a curl handle */
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            break;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        //
        if (timeout_conn_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_conn_ms);
        }
        if (timeout_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        }

        /* Now specify the POST data */

        if (headers)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        if (bodies.length() > 0)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodies.c_str());
            if (en_debug)
            {
                os_log_printf(OS_LOG_DEBUG, "curl", "(len=%d) %s", bodies.length(), bodies.c_str());
            }
        }

        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userdata);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_perform() failed: %d,%s", res, curl_easy_strerror(res));
            break;
        }

        ret = 0;
    } while (0);

    if (headers)
    {
        curl_slist_free_all(headers);
    }

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

    struct curl_slist* headers = NULL;
    //
    if (buffer.size() == 0)
    {
        // allocate buffer if not
        buffer.resize(4096);
    }
    //
    CURL_WRAPPER(curl, en_debug);
    CURLcode res;

    CurlHttpWriteData userdata;
    userdata.buf   = buffer.data();
    userdata.bufsz = buffer.size();
    userdata.wc    = 0;
    memset(userdata.buf, 0, userdata.bufsz);

    for (auto& e : header)
    {
        auto obj = e.first + ": " + e.second;
        if (!headers)
        {
            headers = curl_slist_append(NULL, obj.c_str());
        } else
        {
            curl_slist_append(headers, obj.c_str());
        }
    }

    do
    {
        /* get a curl handle */
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            break;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        //
        if (timeout_conn_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_conn_ms);
        }
        if (timeout_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        }

        /* Now specify the POST data */

        if (headers)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        // if (bodies.length() > 0)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodies.c_str());
            if (en_debug)
            {
                os_log_printf(OS_LOG_DEBUG, "curl", "(len=%d) %s", bodies.length(), bodies.c_str());
            }
        }



        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userdata);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_perform() failed: %d,%s", res, curl_easy_strerror(res));
            break;
        }

        ret = 0;
    } while (0);

    if (headers)
    {
        curl_slist_free_all(headers);
    }


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
    if (header.find("Content-Type") != header.end())
    {
        header["Content-Type"] = "application/x-www-form-urlencoded;charset=utf-8";
    }

    std::string bodies;

    for (auto& e : body)
    {
        bodies += e.first + "=" + e.second + "&";
    }
    if (bodies.length() > 0)
    {
        if (bodies.at(bodies.length() - 1) == '&')
        {
            bodies.at(bodies.length() - 1) = '\0';
        }
    }
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
    if (header.find("Content-Type") != header.end())
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
