#include "qrcode_content.hpp"

#include <cstdio>
#include <string>

#include "utils/os_tools_log.h"

int qrcode_wifi_content_write(char *buf, int sz, const std::string &info, bool escape = true)
{
    if (!buf || sz <= 0)
    {
        OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
        return -1;
    }

    auto dst = buf;
    //
    auto dst_end = dst + sz - 1;
    //
    auto ptr0 = info.c_str();
    //
    auto ptr_end = info.c_str() + info.length();
    //
    for (; ptr0 != ptr_end && dst != dst_end; ++ptr0, ++dst)
    {
        if (escape)
        {
            if (ptr0[0] == ';' || ptr0[0] == ':' || ptr0[0] == '\\')
            {
                *dst = '\\';
                //
                if (++dst == dst_end)
                {
                    OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
                    break;
                }
            }
        }
        *dst = *ptr0;
    }
    *dst = '\0';
    //
    int wc = dst - buf;
    //
    return wc;
}

int qrcode_wifi_content_append(char *buf, int sz, const std::string &key, const std::string &value, bool escape = true)
{
    //
    int offset = 0, wc = 0;
    // write key
    wc = qrcode_wifi_content_write(buf + offset, sz - offset, key, escape);
    if (wc < 0)
    {
        OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
        return -1;
    }
    offset += wc;

    // write ":"
    wc = snprintf(buf + offset, sz - offset, ":");
    if (wc < 0)
    {
        OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
        return -1;
    }
    offset += wc;

    // write "value"
    wc = qrcode_wifi_content_write(buf + offset, sz - offset, value, escape);
    if (wc < 0)
    {
        OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
        return -1;
    }
    offset += wc;

    // write ";"
    wc = snprintf(buf + offset, sz - offset, ";");
    if (wc < 0)
    {
        OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
        return -1;
    }
    offset += wc;

    return offset;
}


int gen_qrcode_wifi_content(std::string &outstring, const std::string &SSID, const std::string &PSK, EnQrCodeWifiEncryptFmt fmt, bool hidden)
{
    int ret = -1;
    //
    std::string info;
    {
        char S[48];
        char P[48];
        char T[16];
        char H[16];

        ret = qrcode_wifi_content_append(S, sizeof(S), "S", SSID, true);
        if (ret < 0)
        {
            OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
            return ret;
        }

        ret = qrcode_wifi_content_append(P, sizeof(P), "P", PSK, true);
        if (ret < 0)
        {
            OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
            return ret;
        }

        ret = qrcode_wifi_content_append(T,
                                         sizeof(T),
                                         "T",
                                         fmt == QRCODE_WIFI_ENCRYPT_NONE   ? "nopass"
                                         : fmt == QRCODE_WIFI_ENCRYPT_WEP  ? "WEP"
                                         : fmt == QRCODE_WIFI_ENCRYPT_WPA  ? "WPA"
                                         : fmt == QRCODE_WIFI_ENCRYPT_WPA2 ? "WPA2"
                                         : PSK.length() >= 8               ? "WPA2"
                                                                           : "nopass",
                                         true);
        if (ret < 0)
        {
            OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
            return ret;
        }

        ret = qrcode_wifi_content_append(H, sizeof(H), "H", hidden ? "true" : "false");
        if (ret < 0)
        {
            OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
            return ret;
        }

        info = std::string(S) + std::string(P) + std::string(T);

        // hidden feature maybe not supported by IOS, so only append if it is hidden.
        if (hidden)
        {
            info += H;
        }
    }


    {
        //
        char content[128];
        ret = qrcode_wifi_content_append(content, sizeof(content), "WIFI", info, false);
        if (ret < 0)
        {
            OS_LOGD("<%s,%d>", __FUNCTION__, __LINE__);
            return ret;
        }

        outstring = content;
    }

    return ret;
}
