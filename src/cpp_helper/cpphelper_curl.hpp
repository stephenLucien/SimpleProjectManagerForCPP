#ifndef __CPPHELPER_CURL_H__
#define __CPPHELPER_CURL_H__


#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>


//
#include <curl/curl.h>
#include "cpp_helper/cpphelper_nlohmann_json.hpp"
#include "cpp_helper/cpphelper_scanJsonStr.hpp"
#include "nlohmann/json.hpp"

//
#include "utils/os_tools.h"



class CurlSlist
{
   private:
    struct curl_slist* headers = NULL;

   public:
    CurlSlist(const std::unordered_map<std::string, std::string>& header = std::unordered_map<std::string, std::string>())
    {
        append(header);
    }
    virtual ~CurlSlist()
    {
        cleanup();
    }

    void append(const std::unordered_map<std::string, std::string>& header)
    {
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
    }

    void cleanup()
    {
        if (headers)
        {
            curl_slist_free_all(headers);
            headers = NULL;
        }
    }

    struct curl_slist* ptr()
    {
        return headers;
    }
};

/**
 * @brief Init and clean up
 *
 */
class CurlWrapper
{
   private:
    //
    typedef struct
    {
        uint8_t* buf;
        size_t   bufsz;
        size_t   wc;
    } CurlHttpWriteData;
    //
    typedef struct
    {
        size_t buf_header_out_offset;
        char   buf_header_out[1020];
        //
        size_t buf_data_out_offset;
        char   buf_data_out[4096];
        //
        size_t buf_header_in_offset;
        char   buf_header_in[1024];
        //
        size_t buf_data_in_offset;
        char   buf_data_in[1024 * 10];
    } DebugData;
    //
    CURL* curl = NULL;
    //
    CurlSlist headers;
    //
    FILE* fd = NULL;
    //
    CurlHttpWriteData wbuf;
    //
    DebugData* p_debug_data = NULL;
    //
    float m_download_progress = 0;
    float m_upload_progress   = 0;

    //
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

    //
    static void dump_debug_data(DebugData* pdata)
    {
        if (!pdata)
        {
            return;
        }
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "header out: \n%s", pdata->buf_header_out);
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "data out: \n%s", pdata->buf_data_out);
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "header in: \n%s", pdata->buf_header_in);
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "data in: \n%s", pdata->buf_data_in);
    }

    //
    static int debug_func(CURL* handle, curl_infotype type, char* data, size_t size, void* clientp)
    {
        auto* pdata = (DebugData*)clientp;
        if (!pdata)
        {
            return -1;
        }
        char*   ptr     = NULL;
        size_t* poffset = NULL;
        size_t  sz      = 0;
        switch (type)
        {
            case CURLINFO_HEADER_OUT:
            {
                poffset = &pdata->buf_header_out_offset;
                ptr     = pdata->buf_header_out + pdata->buf_header_out_offset;
                sz      = sizeof(pdata->buf_header_out) - pdata->buf_header_out_offset;
            }
            break;
            case CURLINFO_DATA_OUT:
            {
                poffset = &pdata->buf_data_out_offset;
                ptr     = pdata->buf_data_out + pdata->buf_data_out_offset;
                sz      = sizeof(pdata->buf_data_out) - pdata->buf_data_out_offset;
            }
            break;
            case CURLINFO_HEADER_IN:
            {
                poffset = &pdata->buf_header_in_offset;
                ptr     = pdata->buf_header_in + pdata->buf_header_in_offset;
                sz      = sizeof(pdata->buf_header_in) - pdata->buf_header_in_offset;
            }
            break;
            case CURLINFO_DATA_IN:
            {
                poffset = &pdata->buf_data_in_offset;
                ptr     = pdata->buf_data_in + pdata->buf_data_in_offset;
                sz      = sizeof(pdata->buf_data_in) - pdata->buf_data_in_offset;
            }
            break;
            default: /* in case a new one is introduced to shock us */
                return 0;
        }
        if (!ptr || !poffset || sz <= 1)
        {
            return 0;
        }
        sz -= 1;
        //
        size_t cpsz = size > sz ? sz : size;
        //
        memcpy(ptr, data, cpsz);
        //
        *poffset += cpsz;
        //
        return 0;
    }

    //
    static void setup_curl_debug(CURL* curl, DebugData* pdata)
    {
        if (!curl)
        {
            return;
        }
        /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_func);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, pdata);
    }

    //
    static int m_curl_xferinfo_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        auto* pobj = (CurlWrapper*)clientp;
        if (!pobj)
        {
            return -1;
        }
#if 0
        char buftest[1024];
        sprintf(buftest, "\n process: dltotal=%ld,dlnow=%ld,ultotal=%ld,ulnow=%ld", dltotal, dlnow, ultotal, ulnow);
        OS_LOGV("%s", buftest);
#endif
        pobj->m_download_progress = dltotal ? (100.0 * dlnow / dltotal) : 0;
        pobj->m_upload_progress   = ultotal ? (100.0 * ulnow / ultotal) : 0;

        return 0;
    }
    //
    int _deinit()
    {
        if (curl)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }
        if (p_debug_data)
        {
            dump_debug_data(p_debug_data);
            //
            free(p_debug_data);
            p_debug_data = NULL;
        }
        if (fd)
        {
            fclose(fd);
            fd = NULL;
        }
        return 0;
    }
    //
    int _init(bool dumpInfo = false)
    {
        _deinit();
        //
        curl = curl_easy_init();
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, m_curl_xferinfo_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);

        if (dumpInfo)
        {
            p_debug_data = (DebugData*)malloc(sizeof(DebugData));
            //
            memset(p_debug_data, 0, sizeof(DebugData));
            //
            setup_curl_debug(curl, p_debug_data);
        }
        return 0;
    }



   public:
    CurlWrapper(bool dumpInfo = false)
    {
        _init(dumpInfo);
    }
    virtual ~CurlWrapper()
    {
        _deinit();
    }
    //
    bool isValid()
    {
        return curl ? true : false;
    }

    CURL* ptr()
    {
        return curl;
    }

    static int perform(CURL* curl)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        ret = curl_easy_perform(curl);
        if (ret != CURLE_OK)
        {
        }

        return 0;
    }
    int perform()
    {
        return perform(curl);
    }

    float getUploadProgress()
    {
        return m_upload_progress;
    }
    float getDownloadProgress()
    {
        return m_download_progress;
    }

    static int getLength(CURL* curl, double& length_sz)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        ret = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length_sz);
        if (ret != CURLE_OK)
        {
        }

        return 0;
    }
    int getLength(double& length_sz)
    {
        return getLength(curl, length_sz);
    }

    static int setUrl(CURL* curl, const std::string& url, int followLocation = 1, int ssl_verifypeer = 0)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, ssl_verifypeer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followLocation);

        return 0;
    }

    int setUrl(const std::string& url, int followLocation = 1, int ssl_verifypeer = 0)
    {
        return setUrl(curl, url, followLocation, ssl_verifypeer);
    }

    static int setUrl(
        CURL* curl, const std::string& url, const std::unordered_map<std::string, std::string>& query, int followLocation = 1, int ssl_verifypeer = 0)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        auto surfix  = genFormDataString(query, true);
        auto new_url = url;
        if (!surfix.empty())
        {
            new_url += "?";
            new_url += surfix;
        }
        curl_easy_setopt(curl, CURLOPT_URL, new_url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, ssl_verifypeer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followLocation);


        return 0;
    }
    int setUrl(const std::string& url, const std::unordered_map<std::string, std::string>& query, int followLocation = 1, int ssl_verifypeer = 0)
    {
        return setUrl(curl, url, query, followLocation, ssl_verifypeer);
    }

    static int setupTimeout(CURL* curl, int timeout_ms = -1, int timeout_conn_ms = 5 * 1000)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        //
        if (timeout_conn_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_conn_ms);
        }
        if (timeout_ms > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        }
        return 0;
    }
    int setupTimeout(int timeout_ms = -1, int timeout_conn_ms = 5 * 1000)
    {
        return setupTimeout(curl, timeout_ms, timeout_conn_ms);
    }

    int setupWriteFile(const std::string& path)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        if (fd)
        {
            fclose(fd);
            fd = NULL;
        }
        fd = fopen(path.c_str(), "wb");
        if (!fd)
        {
            return ret;
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);

        return 0;
    }

    int setupWriteBuffer(void* buf, size_t bufsz)
    {
        int ret = -1;
        //
        if (!buf || bufsz <= 1)
        {
            return ret;
        }
        memset(buf, 0, bufsz);
        //
        if (!curl)
        {
            return ret;
        }
        memset(&wbuf, 0, sizeof(wbuf));
        wbuf.buf   = (uint8_t*)buf;
        wbuf.bufsz = bufsz - 1;
        //
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wbuf);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);

        //
        return 0;
    }

    static int setMethod_DELETE(CURL* curl)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        return 0;
    }
    int setMethod_DELETE()
    {
        return setMethod_DELETE(curl);
    }

    static int setMethod_GET(CURL* curl)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        return 0;
    }
    int setMethod_GET()
    {
        return setMethod_GET(curl);
    }

    static int setMethod_POST(CURL* curl)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        curl_easy_setopt(curl, CURLOPT_POST, 1);

        return 0;
    }
    int setMethod_POST()
    {
        return setMethod_POST(curl);
    }

    static int setPOSTFIELDS(CURL* curl, const void* ptr, int len)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ptr);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);

        return 0;
    }
    int setPOSTFIELDS(const void* ptr, int len)
    {
        return setPOSTFIELDS(curl, ptr, len);
    }

    static std::string genFormDataString(const std::unordered_map<std::string, std::string>& formDatas, bool b_url_escape = false)
    {
        std::string queries;
        //
        int cnt = 0;
        //
        for (auto& e : formDatas)
        {
            auto tmpkey = e.first;
            auto tmpval = e.second;
            //
            char* ptr_key = NULL;
            char* ptr_val = NULL;
            if (b_url_escape)
            {
                ptr_key = curl_escape(tmpkey.c_str(), tmpkey.length());
                ptr_val = curl_escape(tmpval.c_str(), tmpval.length());
            }

            if (cnt)
            {
                queries += "&";
            }
            queries += (ptr_key ? std::string(ptr_key) : e.first) + "=" + (ptr_val ? std::string(ptr_val) : e.second);

            ++cnt;
            if (ptr_key)
            {
                curl_free(ptr_key);
            }
            if (ptr_val)
            {
                curl_free(ptr_val);
            }
        }

        return queries;
    }

    static int setupHeaders(CURL* curl, CurlSlist& slistHeaders)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }

        if (slistHeaders.ptr())
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slistHeaders.ptr());
        }

        return 0;
    }

    int setupHeaders(const std::unordered_map<std::string, std::string>& header)
    {
        int ret = -1;
        //
        if (!curl)
        {
            return ret;
        }
        headers.cleanup();
        headers.append(header);
        return setupHeaders(curl, headers);
    }
};


class ScanJsonStringCurl
{
   private:
    //
    ScanJsonStringInplace scanner;
    //
    size_t buf_data_in_offset;
    //
    std::vector<char> buf_data_in;
    //
    void (*js_handler)(void* d, char* js) = NULL;
    void* js_handler_userdata             = NULL;
    //
    //
    static size_t curl_http_write_cb(void* data, size_t nsz, size_t nmemb, void* clientp)
    {
        auto* pobj = (ScanJsonStringCurl*)clientp;
        if (!pobj)
        {
            return -1;
        }
        auto size = nsz * nmemb;
        //
        char*   ptr     = NULL;
        size_t* poffset = NULL;
        size_t  sz      = 0;
        //
        poffset = &pobj->buf_data_in_offset;
        ptr     = pobj->buf_data_in.data() + pobj->buf_data_in_offset;
        sz      = pobj->buf_data_in.size() - pobj->buf_data_in_offset;
        //
        if (!ptr || !poffset || sz <= 1)
        {
            return 0;
        }
        //
        sz -= 1;
        //
        size_t cpsz = size > sz ? sz : size;
        //
        memcpy(ptr, data, cpsz);
        //
        if (*poffset == 0)
        {
            pobj->scanner.init(ptr);
        }
        //
        *poffset += cpsz;
        //
        size_t scan_cnt = 0;
        //
        do
        {
            char* js_begin = NULL;
            char* js_end   = NULL;
            //
            auto cnt = pobj->scanner.scan(cpsz - scan_cnt, &js_begin, &js_end);
            if (js_begin)
            {
                auto len = js_end + 1 - js_begin;
                //
                std::vector<char> tmpBuf(len + 1);
                //
                memcpy(tmpBuf.data(), js_begin, len);
                tmpBuf.data()[len] = '\0';
                //
                OS_LOGV("%s", tmpBuf.data());


                if (pobj->js_handler)
                {
                    pobj->js_handler(pobj->js_handler_userdata, tmpBuf.data());
                } else
                {
                    //
                    auto js = nlohmann_js_parse(tmpBuf.data());
                    //
                    OS_LOGD("%s", js.dump(4).c_str());
                }
            }
            scan_cnt += cnt;
        } while (scan_cnt < cpsz);

        return size;
    }

   public:
    ScanJsonStringCurl()
    {
    }
    ScanJsonStringCurl(CurlWrapper& curl_cpp, size_t bufsz = 128 * 1024)
    {
        setup_curl(curl_cpp.ptr(), bufsz);
    }
    char* getResp()
    {
        return buf_data_in.data();
    }
    void dumpResp()
    {
        //
        OS_PRINT("data in: \n%s", getResp());
    }
    void setup_hdl(void (*JsHdl)(void* d, char* js) = NULL, void* d = NULL)
    {
        js_handler          = JsHdl;
        js_handler_userdata = d;
    }
    void setup_curl(CURL* curl, size_t bufsz = 128 * 1024)
    {
        if (!curl)
        {
            return;
        }
        //
        buf_data_in_offset = 0;
        buf_data_in.resize(bufsz);
        memset(buf_data_in.data(), 0, buf_data_in.size());
        //

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_http_write_cb);
    }
};


#define DEFAULT_CURL_CONNECT_TIMEOUT_MS (5 * 1000)
#define DEFAULT_CURL_TIMEOUT_MS         (10 * 1000)

/**
 * @brief
 *
 * @param [in] url
 * @param [in] outputfile
 * @param timeout_conn_ms : Time-out connect operations after this amount of
 miliseconds, if connects are OK within this time, then fine... This only aborts
 the connect phase.
 * - 0 : no timeout
 * - positive : timeout connect url
 * @param timeout_ms : Time-out the read operation after this amount of
 miliseconds
 * - 0 : no timeout
 * - positive : timeout in miliseconds
 * @param en_debug
 * @return int :
 * - 0 : success
 * - other : err
 */
int curl_download_file(
    const char* url, const char* outputfile, int timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS, int timeout_ms = 0, int en_debug = 0);

/**
 * @brief
 *
 * @param url
 * @param header
 * @param body
 * @param buffer
 * @param timeout_conn_ms : Time-out connect operations after this amount of
 miliseconds, if connects are OK within this time, then fine... This only aborts
 the connect phase.
 * - 0 : no timeout
 * - positive : timeout connect url
 * @param timeout_ms : Time-out the read operation after this amount of
 miliseconds
 * - 0 : no timeout
 * - positive : timeout in miliseconds
 * @return int :
 * - 0 : success
 * - other : err
 */
int curl_delete_basic(const std::string&                                  url,
                      const std::unordered_map<std::string, std::string>& header,
                      const std::unordered_map<std::string, std::string>& body,
                      std::vector<uint8_t>&                               buffer,
                      int                                                 timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS,
                      int                                                 timeout_ms      = DEFAULT_CURL_TIMEOUT_MS,
                      int                                                 en_debug        = 0);

/**
 * @brief
 *
 * @param url
 * @param header
 * @param query
 * @param buffer
 * @param timeout_conn_ms : Time-out connect operations after this amount of
 miliseconds, if connects are OK within this time, then fine... This only aborts
 the connect phase.
 * - 0 : no timeout
 * - positive : timeout connect url
 * @param timeout_ms : Time-out the read operation after this amount of
 miliseconds
 * - 0 : no timeout
 * - positive : timeout in miliseconds
 * @return int :
 * - 0 : success
 * - other : err
 */
int curl_get_basic(std::string                                         url,
                   const std::unordered_map<std::string, std::string>& header,
                   const std::unordered_map<std::string, std::string>& query,
                   std::vector<uint8_t>&                               buffer,
                   int                                                 timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS,
                   int                                                 timeout_ms      = DEFAULT_CURL_TIMEOUT_MS,
                   int                                                 en_debug        = 0);

/**
 * @brief
 *
 * @param url
 * @param header
 * @param bodies
 * @param buffer
 * @param timeout_conn_ms : Time-out connect operations after this amount of
 miliseconds, if connects are OK within this time, then fine... This only aborts
 the connect phase.
 * - 0 : no timeout
 * - positive : timeout connect url
 * @param timeout_ms : Time-out the read operation after this amount of
 miliseconds
 * - 0 : no timeout
 * - positive : timeout in miliseconds
 * @return int :
 * - 0 : success
 * - other : err
 */
int curl_post_basic(const std::string&                                  url,
                    const std::unordered_map<std::string, std::string>& header,
                    const std::string&                                  bodies,
                    std::vector<uint8_t>&                               buffer,
                    int                                                 timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS,
                    int                                                 timeout_ms      = DEFAULT_CURL_TIMEOUT_MS,
                    int                                                 en_debug        = 0);

/**
 * @brief
 *
 * @param url
 * @param header
 * @param body
 * @param buffer
 * @param timeout_conn_ms : Time-out connect operations after this amount of
 miliseconds, if connects are OK within this time, then fine... This only aborts
 the connect phase.
 * - 0 : no timeout
 * - positive : timeout connect url
 * @param timeout_ms : Time-out the read operation after this amount of
 miliseconds
 * - 0 : no timeout
 * - positive : timeout in miliseconds
 * @return int :
 * - 0 : success
 * - other : err
 */
int curl_post_form_basic(const std::string&                                  url,
                         const std::unordered_map<std::string, std::string>& header,
                         const std::unordered_map<std::string, std::string>& body,
                         std::vector<uint8_t>&                               buffer,
                         int                                                 timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS,
                         int                                                 timeout_ms      = DEFAULT_CURL_TIMEOUT_MS,
                         int                                                 en_debug        = 0);

/**
 * @brief
 *
 * @param url
 * @param header
 * @param body
 * @param buffer
 * @param timeout_conn_ms : Time-out connect operations after this amount of
 miliseconds, if connects are OK within this time, then fine... This only aborts
 the connect phase.
 * - 0 : no timeout
 * - positive : timeout connect url
 * @param timeout_ms : Time-out the read operation after this amount of
 miliseconds
 * - 0 : no timeout
 * - positive : timeout in miliseconds
 * @return int :
 * - 0 : success
 * - other : err
 */
int curl_post_json_basic(const std::string&                                  url,
                         const std::unordered_map<std::string, std::string>& header,
                         const nlohmann::json&                               body,
                         std::vector<uint8_t>&                               buffer,
                         int                                                 timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS,
                         int                                                 timeout_ms      = DEFAULT_CURL_TIMEOUT_MS,
                         int                                                 en_debug        = 0);


#endif  // __CPPHELPER_CURL_H__
