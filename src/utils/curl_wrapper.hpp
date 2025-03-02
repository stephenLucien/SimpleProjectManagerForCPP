#ifndef __CURL_WRAPPER_H__
#define __CURL_WRAPPER_H__

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>


//
#include <curl/curl.h>
#include "nlohmann/json.hpp"

//
#include "os_tools.h"

/**
 * @brief Init and clean up
 *
 */
class CurlWrapper
{
   private:
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
    CURL* curl = NULL;
    //
    DebugData* p_debug_data = NULL;

   public:
    int deinit()
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
        return 0;
    }

    int init(bool dumpInfo = false)
    {
        deinit();
        //
        curl = curl_easy_init();
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            return -1;
        }
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

    CurlWrapper(bool dumpInfo = false)
    {
        init(dumpInfo);
    }
    ~CurlWrapper()
    {
        deinit();
    }

    CURL* ptr()
    {
        return curl;
    }
};

#define CURL_WRAPPER(tag, en_dump)  \
    CurlWrapper tag##_cpp(en_dump); \
    auto        tag = tag##_cpp.ptr();


#define DEFAULT_CURL_CONNECT_TIMEOUT_MS (5 * 1000)
#define DEFAULT_CURL_TIMEOUT_MS         (10 * 1000)

/**
 * @brief
 *
 * @param [in] url
 * @param [in] outputfile
 * @param [out] wc
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
int curl_download_file(const char* url,
                       const char* outputfile,
                       size_t*     wc              = NULL,
                       int         timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS,
                       int         timeout_ms      = 0,
                       int         en_debug        = 0);

/**
 * @brief
 *
 * @param [in] url
 * @param [out] sz
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
int curl_get_file_size(const char* url, size_t* sz, int timeout_conn_ms = DEFAULT_CURL_CONNECT_TIMEOUT_MS, int timeout_ms = 0, int en_debug = 0);

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


#endif  // __CURL_WRAPPER_H__
