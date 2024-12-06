#include "parse_uri_http.h"

//
#include <ctype.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>


static char rfc3986[256] = {0};

int http_url_encode(const char *raw, size_t rawlen, const char *ignore, char *out, size_t outsz)
{
    int ret = -1;
    if (!raw || !out || outsz == 0)
    {
        return ret;
    }
    // init table
    if (rfc3986['0'] == 0)
    {
        for (int i = 0; i < 256; i++)
        {
            rfc3986[i] = isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
        }
    }
    ret = 0;
    //
    const uint8_t *src_itr = (uint8_t *)raw;
    const uint8_t *src_end = src_itr + rawlen;
    //
    uint8_t *dst_itr = (uint8_t *)out;
    uint8_t *dst_end = dst_itr + outsz;
    //
    while (src_itr != src_end && dst_itr != dst_end)
    {
        if (rfc3986[*src_itr])
        {
            *dst_itr++ = *src_itr;
        } else if (ignore && strchr(ignore, *src_itr))
        {
            // discard
            *dst_itr++ = *src_itr;
        } else
        {
            int wc = snprintf((char *)dst_itr, dst_end - dst_itr, "%%%02X", *src_itr);
            //
            if (wc > 0)
            {
                dst_itr += wc;
                //
                ++ret;
            }
        }
        ++src_itr;
    }
    return ret;
}


int parse_uri_http(const char *url, uint8_t *btls, char *host, size_t hostlen, uint16_t *port, char *path, size_t pathlen)
{
    int ret = -1;
    if (!url)
    {
        return ret;
    }

    const char *http_cmp_str  = "http://";
    const char *https_cmp_str = "https://";
    //
    const char *http_str  = strcasestr(url, http_cmp_str);
    const char *https_str = strcasestr(url, https_cmp_str);
    //
    const char *host_begin = NULL;

    if (http_str)
    {
        host_begin = http_str + strlen(http_cmp_str);
    } else if (https_str)
    {
        host_begin = https_str + strlen(https_cmp_str);
    } else
    {
        host_begin = url;
    }

    const char *res_begin = strchr(host_begin, '/');
    const char *host_end  = res_begin;
    if (!host_end)
    {
        host_end = host_begin + strlen(host_begin);
    }

    const char *port_begin = NULL;

    const char *ptr = host_end;
    while (--ptr > host_begin)
    {
        if (*ptr == ':')
        {
            port_begin = ptr;
            break;
        }
    }

    if (port_begin)
    {
        host_end = port_begin;
        port_begin++;
    }


    int     mport    = -1;
    uint8_t got_port = 0;
    if (port_begin && sscanf(port_begin, "%d", &mport) == 1 && mport >= 0 && mport <= UINT16_MAX)
    {
        got_port = 1;
    } else
    {
        if (http_str)
        {
            mport = 80;
        } else if (https_str)
        {
            mport = 443;
        } else
        {
            mport = 80;
        }
    }

    if (port)
    {
        *port = mport;
    }

    uint8_t mtls;
    if (http_str)
    {
        mtls = 0;
    } else if (https_str)
    {
        mtls = 1;
    } else if (mport == 443)
    {
        mtls = 1;
    } else if (mport == 80)
    {
        mtls = 0;
    } else
    {
        mtls = 0;
    }

    if (btls)
    {
        *btls = mtls;
    }

    if (host && hostlen)
    {
        memset(host, 0, hostlen);
        http_url_encode(host_begin, host_end - host_begin, NULL, host, hostlen);
        host[hostlen - 1] = 0;
    }

    if (path && pathlen)
    {
        memset(path, 0, pathlen);
        if (res_begin)
        {
            // snprintf(path, pathlen, "%s", res_begin);
            http_url_encode(res_begin, strlen(res_begin), "/", path, pathlen);
            path[pathlen - 1] = 0;
        }
    }

    return 0;
}
