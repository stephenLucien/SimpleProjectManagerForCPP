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
#include "cpp_helper/cpphelper_pthread.hpp"
#include "cpp_helper/cpphelper_scanJsonStr.hpp"
#include "nlohmann/json.hpp"

//
#include "utils/os_tools_log.h"

#ifndef CURL_WRITEFUNC_ERROR
    #define CURL_WRITEFUNC_ERROR (0xFFFFFFFF)
#endif

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

class CurlReadWriteImpl
{
   public:
    CurlReadWriteImpl()  = default;
    ~CurlReadWriteImpl() = default;

    // TODO: should be implemented by derived class
    virtual bool isValid() = 0;

    virtual int open()
    {
        return 0;
    }

    // TODO: should be implemented by derived class
    virtual bool isReader() = 0;

    virtual size_t write(void* data, size_t nsz, size_t nmemb)
    {
        OS_LOGE("this function should be implemented by writer!");
        return CURL_WRITEFUNC_ERROR;
    }

    virtual size_t read(void* data, size_t nsz, size_t nmemb)
    {
        OS_LOGE("this function should be implemented by reader!");
        return CURL_WRITEFUNC_ERROR;
    }

    virtual int close()
    {
        return 0;
    }

    virtual long getInFileSize()
    {
        OS_LOGE("this function should be implemented by reader!");
        long sz = 0;
        return sz;
    }
};

class CurlSetupReadWrite
{
   private:
    //
    CurlReadWriteImpl* m_io = nullptr;
    //
    bool b_cancel = false;
    //
    bool b_print_progress = false;
    //
    curl_off_t dltotal = 0, dlnow = 0, ultotal = 0, ulnow = 0;
    //
    float m_download_progress = 0;
    //
    float m_upload_progress = 0;
    //
    //
    static int m_curl_xferinfo_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        auto* pobj = (CurlSetupReadWrite*)clientp;
        if (!pobj)
        {
            return -1;
        }
        pobj->dltotal = dltotal;
        pobj->dlnow   = dlnow;
        pobj->ultotal = ultotal;
        pobj->ulnow   = ulnow;
        //
        pobj->m_download_progress = dltotal ? (100.0 * dlnow / dltotal) : 0;
        pobj->m_upload_progress   = ultotal ? (100.0 * ulnow / ultotal) : 0;

        if (pobj->b_print_progress)
        {
            char buftest[1024];
            memset(buftest, 0, sizeof(buftest));
            //
            if (pobj->m_io && pobj->m_io->isReader())
            {
                sprintf(buftest,
                        "ultotal=%" CURL_FORMAT_CURL_OFF_T ", ulnow=%" CURL_FORMAT_CURL_OFF_T ", upro=%f",
                        ultotal,
                        ulnow,
                        pobj->m_upload_progress);
            } else
            {
                //
                sprintf(buftest,
                        "dltotal=%" CURL_FORMAT_CURL_OFF_T ", dlnow=%" CURL_FORMAT_CURL_OFF_T ", dpro=%f",
                        dltotal,
                        dlnow,
                        pobj->m_download_progress);
            }
            OS_LOGV("%s", buftest);
        }
        return 0;
    }

    //
    static size_t curl_fd_write_cb(void* data, size_t nsz, size_t nmemb, void* clientp)
    {
        auto* pobj = (CurlSetupReadWrite*)clientp;
        if (!pobj)
        {
            return CURL_WRITEFUNC_ERROR;
        }
        if (pobj->b_cancel)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        if (!pobj->m_io)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        return pobj->m_io->write(data, nsz, nmemb);
    }

    //
    static size_t curl_fd_read_cb(void* data, size_t nsz, size_t nmemb, void* clientp)
    {
        auto* pobj = (CurlSetupReadWrite*)clientp;
        if (!pobj)
        {
            return CURL_WRITEFUNC_ERROR;
        }
        if (pobj->b_cancel)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        if (!pobj->m_io)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        return pobj->m_io->read(data, nsz, nmemb);
    }

   public:
    CurlSetupReadWrite() = default;
    CurlSetupReadWrite(CURL* curl, CurlReadWriteImpl* pio)
    {
        setupIO(curl, pio);
    }
    int setupIO(CURL* curl, CurlReadWriteImpl* pio)
    {
        if (!curl)
        {
            return -1;
        }
        m_io = pio;
        if (!m_io)
        {
            return -1;
        }
        if (m_io->open() != 0)
        {
            return -1;
        }
        if (!m_io->isValid())
        {
            return -1;
        }
        //
        b_cancel = false;
        if (m_io->isReader())
        {  //
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, curl_fd_read_cb);
            curl_easy_setopt(curl, CURLOPT_READDATA, this);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)m_io->getInFileSize());
        } else
        {
            //
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_fd_write_cb);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        }
        //
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, m_curl_xferinfo_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        return 0;
    }
    int closeIO()
    {
        if (!m_io)
        {
            return -1;
        }
        return m_io->close();
    }
    void setPrint(bool enbale)
    {
        this->b_print_progress = enbale;
    }
    void cancel()
    {
        this->b_cancel = true;
    }

    float getUploadProgress()
    {
        return m_upload_progress;
    }
    long getUploadTotal()
    {
        return ultotal;
    }
    long getUploadNow()
    {
        return ulnow;
    }

    float getDownloadProgress()
    {
        return m_download_progress;
    }
    long getDownloadTotal()
    {
        return dltotal;
    }
    long getDownloadNow()
    {
        return dlnow;
    }
};


class CurlFileWriter : public CurlReadWriteImpl
{
   private:
    //
    FILE* fd = NULL;
    //
    std::string filepath;
    //
    std::string filemode;
    //
    bool bOpenPipe = false;
    //
    int open(const std::string& f, const std::string& m, bool isPipe)
    {
        close();
        if (isPipe)
        {
            fd = popen(f.c_str(), m.c_str());
        } else
        {
            fd = fopen(f.c_str(), m.c_str());
        }
        return fd ? 0 : -1;
    }

   public:
    virtual ~CurlFileWriter()
    {
        close();
    }
    CurlFileWriter(const std::string& f = std::string(), const std::string& m = "wb", bool isPipe = false)
    {
        setFilePath(f);
        setFileMode(m);
        setPipeOrFile(isPipe);
    }
    // Disable copy constructor
    CurlFileWriter(const CurlFileWriter&) = delete;
    // Disable copy
    CurlFileWriter& operator=(const CurlFileWriter&) = delete;

    void setFilePath(const std::string& filepath)
    {
        this->filepath = filepath;
    }
    void setFileMode(const std::string& filemode)
    {
        this->filemode = filemode;
    }
    void setPipeOrFile(bool isPipe)
    {
        this->bOpenPipe = isPipe;
    }

    int open() override
    {
        return open(filepath, filemode, bOpenPipe);
    }
    int close() override
    {
        if (fd)
        {
            fclose(fd);
            fd = NULL;
        }
        return 0;
    }

    FILE* get()
    {
        return fd;
    }

    bool isValid() override
    {
        return get();
    }

    bool isReader() override
    {
        return false;
    }

    size_t write(void* data, size_t nsz, size_t nmemb) override
    {
        if (!fd)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        return fwrite(data, nsz, nmemb, fd);
    }
};


class CurlFileReader : public CurlReadWriteImpl
{
   private:
    //
    FILE* fd = NULL;
    //
    std::string filepath;
    //
    std::string filemode;
    //
    bool bOpenPipe = false;
    //
    int open(const std::string& f, const std::string& m, bool isPipe)
    {
        close();
        if (isPipe)
        {
            fd = popen(f.c_str(), m.c_str());
        } else
        {
            fd = fopen(f.c_str(), m.c_str());
        }
        return fd ? 0 : -1;
    }

   public:
    virtual ~CurlFileReader()
    {
        close();
    }
    CurlFileReader(const std::string& f = std::string(), const std::string& m = "rb", bool isPipe = false)
    {
        setFilePath(f);
        setFileMode(m);
        setPipeOrFile(isPipe);
    }
    // Disable copy constructor
    CurlFileReader(const CurlFileReader&) = delete;
    // Disable copy
    CurlFileReader& operator=(const CurlFileReader&) = delete;

    void setFilePath(const std::string& filepath)
    {
        this->filepath = filepath;
    }
    void setFileMode(const std::string& filemode)
    {
        this->filemode = filemode;
    }
    void setPipeOrFile(bool isPipe)
    {
        this->bOpenPipe = isPipe;
    }
    int open() override
    {
        return open(filepath, filemode, bOpenPipe);
    };
    int close() override
    {
        if (fd)
        {
            fclose(fd);
            fd = NULL;
        }
        return 0;
    }
    FILE* get()
    {
        return fd;
    }

    bool isValid() override
    {
        return get();
    }

    bool isReader() override
    {
        return true;
    }

    size_t read(void* data, size_t nsz, size_t nmemb) override
    {
        if (!fd)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        return fread(data, nsz, nmemb, fd);
    }


    int getFileSize(const std::string& filename, long& sz)
    {
        int ret = -1;
        //
        auto fn = filename.c_str();
        //
        if (access(fn, F_OK) != 0)
        {
            return ret;
        }
        //
        auto fd = get();
        if (!fd)
        {
            OS_LOGE("open file fail: %s", fn);
            return ret;
        }
        do
        {
            if (fseek(fd, 0, SEEK_END) != 0)
            {
                OS_LOGE("fseek end fail:%s", fn);
                break;
            }
            sz = ftell(fd);
            if (sz < 0)
            {
                OS_LOGE("ftell fail:%s", fn);
                break;
            }
            if (fseek(fd, 0, SEEK_SET) != 0)
            {
                OS_LOGE("fseek set fail:%s", fn);
                break;
            }
            ret = 0;
        } while (0);
        return ret;
    }
    long getInFileSize() override
    {
        long sz = 0;
        getFileSize(filepath, sz);
        return sz;
    }
};

class CurlBufferWriter : public CurlReadWriteImpl
{
   private:
    std::vector<uint8_t> buffer;
    //
    uint8_t* buf = NULL;
    //
    size_t bufsz = 0;
    //
    size_t offset = 0;

   public:
    CurlBufferWriter(size_t bufsz = 1024 * 128)
    {
        this->bufsz = bufsz;
    }
    int open() override
    {
        setupBufferSize(bufsz);
        clearBuffer();
        return 0;
    }
    void setupBufferSize(size_t bufsz = 1024 * 128)
    {
        buffer.resize(bufsz);
        this->buf   = buffer.data();
        this->bufsz = buffer.size();
    }
    void clearBuffer()
    {
        if (!isValid())
        {
            return;
        }
        memset(getBuffer(), 0, getBufferSz());
        //
        offset = 0;
    }
    uint8_t* getBuffer()
    {
        return buf;
    }
    size_t getBufferSz()
    {
        return bufsz;
    }

    bool isValid() override
    {
        return getBuffer() && getBufferSz() > 0;
    }

    bool isReader() override
    {
        return false;
    }

    size_t write(void* data, size_t nsz, size_t nmemb) override
    {
        auto len = nsz * nmemb;
        //
        if (!isValid())
        {
            return CURL_WRITEFUNC_ERROR;
        }
        //
        if (!(offset + len <= getBufferSz()))
        {
            OS_LOGE("buf small");
            return CURL_WRITEFUNC_ERROR;
        }
        auto dst = getBuffer() + offset;
        auto src = (uint8_t*)data;
        memcpy(dst, src, len);
        offset += len;
        return len;
    }
};


class CurlExternalBufferWriter : public CurlReadWriteImpl
{
   private:
    //
    uint8_t* buf = NULL;
    //
    size_t bufsz = 0;
    //
    size_t offset = 0;

   public:
    CurlExternalBufferWriter(uint8_t* buf, size_t bufsz)
    {
        setupExternalBuffer(buf, bufsz);
    }
    int open() override
    {
        clearBuffer();
        return 0;
    }
    void setupExternalBuffer(uint8_t* buf, size_t bufsz)
    {
        this->buf   = buf;
        this->bufsz = bufsz;
    }
    void clearBuffer()
    {
        if (!isValid())
        {
            return;
        }
        memset(getBuffer(), 0, getBufferSz());
        //
        offset = 0;
    }
    uint8_t* getBuffer()
    {
        return buf;
    }
    size_t getBufferSz()
    {
        return bufsz;
    }

    bool isValid() override
    {
        return getBuffer() && getBufferSz() > 0;
    }

    bool isReader() override
    {
        return false;
    }

    size_t write(void* data, size_t nsz, size_t nmemb) override
    {
        auto len = nsz * nmemb;
        //
        if (!isValid())
        {
            return CURL_WRITEFUNC_ERROR;
        }
        //
        if (!(offset + len <= getBufferSz()))
        {
            OS_LOGE("buf small");
            return CURL_WRITEFUNC_ERROR;
        }
        auto dst = getBuffer() + offset;
        auto src = (uint8_t*)data;
        memcpy(dst, src, len);
        offset += len;
        return len;
    }
};

class CurlSetupDebug
{
   private:
    //
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


    //
    static int debug_func(CURL* handle, curl_infotype type, char* data, size_t size, void* clientp)
    {
        auto* pobj = (CurlSetupDebug*)clientp;
        if (!pobj)
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
                poffset = &pobj->buf_header_out_offset;
                ptr     = pobj->buf_header_out + pobj->buf_header_out_offset;
                sz      = sizeof(pobj->buf_header_out) - pobj->buf_header_out_offset;
            }
            break;
            case CURLINFO_DATA_OUT:
            {
                poffset = &pobj->buf_data_out_offset;
                ptr     = pobj->buf_data_out + pobj->buf_data_out_offset;
                sz      = sizeof(pobj->buf_data_out) - pobj->buf_data_out_offset;
            }
            break;
            case CURLINFO_HEADER_IN:
            {
                poffset = &pobj->buf_header_in_offset;
                ptr     = pobj->buf_header_in + pobj->buf_header_in_offset;
                sz      = sizeof(pobj->buf_header_in) - pobj->buf_header_in_offset;
            }
            break;
            case CURLINFO_DATA_IN:
            {
                poffset = &pobj->buf_data_in_offset;
                ptr     = pobj->buf_data_in + pobj->buf_data_in_offset;
                sz      = sizeof(pobj->buf_data_in) - pobj->buf_data_in_offset;
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

   public:
    //
    void setup(CURL* curl)
    {
        if (!curl)
        {
            return;
        }
        /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_func);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, this);
    }

    //
    void dump()
    {
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "header out: \n%s", this->buf_header_out);
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "data out: \n%s", this->buf_data_out);
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "header in: \n%s", this->buf_header_in);
        //
        os_log_printf(OS_LOG_DEBUG, "curl", "data in: \n%s", this->buf_data_in);
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
    CURL* curl = NULL;

    //
    int _deinit()
    {
        if (curl)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }
        return 0;
    }
    //
    int _init()
    {
        _deinit();
        //
        curl = curl_easy_init();
        if (!curl)
        {
            os_log_printf(OS_LOG_ERR, "curl", "curl_easy_init");
            return -1;
        }
        return 0;
    }



   public:
    CurlWrapper()
    {
        _init();
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

    static int perform(CURL* curl, int64_t* p_http_code = NULL)
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
        } else
        {
            int64_t http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (p_http_code)
            {
                *p_http_code = http_code;
            }
        }

        return 0;
    }
    int perform(int64_t* p_http_code = NULL)
    {
        return perform(curl, p_http_code);
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
    int setupHeaders(CurlSlist& slistHeaders)
    {
        return setupHeaders(curl, slistHeaders);
    }
};


class CurlFileDownloader : public PthreadWrapper
{
   private:
    //
    int m_errcode = -1;
    //
    int64_t m_http_code = 0;
    //
    bool m_downloaded = false;
    //
    std::string m_filepath;
    bool        b_tofile = false;
    //
    uint8_t* m_buf   = NULL;
    size_t   m_bufsz = 0;
    bool     b_tobuf = false;
    //
    CurlSetupReadWrite m_writer;
    //
    std::string url;
    //
    int timeout_ms, timeout_conn_ms;
    //
    void resetState()
    {
        //
        m_errcode = -1;
        //
        m_http_code = 0;
        //
        m_downloaded = false;
    }
    //
    bool readyToRun() override
    {
        //
        resetState();
        //
        return true;
    }
    //
    bool threadLoop() override
    {
        if (exitPending())
        {
            return false;
        }
        download();
        return false;
    }

   public:
    CurlFileDownloader() = default;
    CurlFileDownloader(const std::string& url, const std::string& filepath, int timeout_ms = -1, int timeout_conn_ms = -1)
    {
        setupFile(filepath);
        setupUrl(url);
        setupTimeout(timeout_ms);
        setupConnTimeout(timeout_conn_ms);
        //
        resetState();
    }
    CurlFileDownloader(const std::string& url, void* buf, size_t bufsz, int timeout_ms = -1, int timeout_conn_ms = -1)
    {
        setupBuf(buf, bufsz);
        setupUrl(url);
        setupTimeout(timeout_ms);
        setupConnTimeout(timeout_conn_ms);
        //
        resetState();
    }

    void setupUrl(const std::string& url)
    {
        this->url = url;
    }
    void setupTimeout(int timeout_ms)
    {
        this->timeout_ms = timeout_ms;
    }
    void setupConnTimeout(int timeout_conn_ms)
    {
        this->timeout_conn_ms = timeout_conn_ms;
    }

    void setupFile(const std::string& filepath)
    {
        m_filepath = filepath;
        b_tofile   = true;
        b_tobuf    = false;
    }
    void setupBuf(void* buf, size_t bufsz)
    {
        m_buf    = (uint8_t*)buf;
        m_bufsz  = bufsz;
        b_tofile = false;
        b_tobuf  = true;
    }
    //
    int download(const std::string& url, CurlReadWriteImpl& writer, int timeout_ms, int timeout_conn_ms)
    {
        //
        resetState();
        //
        CurlWrapper m_curl_ctx;
        //
        auto curl = m_curl_ctx.ptr();
        if (!curl)
        {
            return -1;
        }
        //
        m_writer.setupIO(m_curl_ctx.ptr(), &writer);
        //
        CurlWrapper::setUrl(curl, url);
        //
        CurlWrapper::setupTimeout(curl, timeout_ms, timeout_conn_ms);
        //
        m_errcode = CurlWrapper::perform(curl, &m_http_code);
        //
        m_writer.closeIO();
        //
        if (m_errcode == 0 && (m_http_code == 0 || m_http_code == 200))
        {
            m_downloaded = true;
        }

        return m_downloaded ? 0 : -1;
    }
    //
    int download(const std::string& filepath)
    {
        CurlFileWriter writer(filepath);
        OS_LOGV("tofile: %s", filepath.c_str());
        return download(url, writer, timeout_ms, timeout_conn_ms);
    }
    //
    int download(void* buf, size_t bufsz)
    {
        CurlExternalBufferWriter writer((uint8_t*)buf, bufsz);
        OS_LOGV("tobuf: %p, sz:%zu", buf, bufsz);
        return download(url, writer, timeout_ms, timeout_conn_ms);
    }
    //
    int download()
    {
        int ret = -1;
        //
        resetState();
        //
        if (b_tofile)
        {
            ret = download(m_filepath);
        } else if (b_tobuf)
        {
            ret = download(m_buf, m_bufsz);
        } else
        {
            ret = -1;
        }
        return ret;
    }
    //
    void requestExit() override
    {
        m_writer.cancel();
    }

    int getErrcode()
    {
        return m_errcode;
    }
    int64_t getHttpCode()
    {
        return m_http_code;
    }
    bool isDownloaded()
    {
        return m_downloaded;
    }

    CurlSetupReadWrite* getWriter()
    {
        return &m_writer;
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
