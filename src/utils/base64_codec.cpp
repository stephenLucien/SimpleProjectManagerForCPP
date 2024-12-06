#include "base64_codec.h"

//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>
#include <cstdint>

static uint8_t b64_dec_table[256] = {0};
//
static const char b64_enc_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char b64_padchar     = '=';
//
static char b64_dec_table_is_inited = 0;
//
static void b64_dec_table_init(void)
{
    if (b64_dec_table_is_inited)
    {
        return;
    }
    memset(b64_dec_table, 0, sizeof(b64_dec_table));
    for (size_t i = 0; i < strlen(b64_enc_table); ++i)
    {
        b64_dec_table[b64_enc_table[i]] = i;
    }
    b64_dec_table_is_inited = 1;
}

static void base64_encode_3bytes(char *b64, const uint8_t *data)
{
    uint32_t triple = (data[0] << 16) | (data[1] << 8) | data[2];
    //
    b64[0] = b64_enc_table[(triple >> 18) & 0x3F];
    b64[1] = b64_enc_table[(triple >> 12) & 0x3F];
    b64[2] = b64_enc_table[(triple >> 6) & 0x3F];
    b64[3] = b64_enc_table[(triple >> 0) & 0x3F];
}

static void base64_encode_1bytes(char *b64, const uint8_t *data)
{
    uint8_t tmp_data[] = {data[0], 0, 0};
    base64_encode_3bytes(b64, tmp_data);
    b64[2] = b64_padchar;
    b64[3] = b64_padchar;
}

static void base64_encode_2bytes(char *b64, const uint8_t *data)
{
    uint8_t tmp_data[] = {data[0], data[1], 0};
    base64_encode_3bytes(b64, tmp_data);
    b64[3] = b64_padchar;
}

static void base64_decode_4char(uint8_t *data, const char *b64)
{
    uint32_t group_a = b64_dec_table[b64[0]];
    uint32_t group_b = b64_dec_table[b64[1]];
    uint32_t group_c = b64_dec_table[b64[2]];
    uint32_t group_d = b64_dec_table[b64[3]];
    //
    uint32_t triple = ((group_a & 0x3F) << 18) + ((group_b & 0x3F) << 12) + ((group_c & 0x3F) << 6) + ((group_d & 0x3F) << 0);
    //
    data[0] = ((triple >> 16) & 0xFF);
    data[1] = ((triple >> 8) & 0xFF);
    data[2] = ((triple >> 0) & 0xFF);
}

static size_t base64_encode_min_sz(size_t data_len)
{
    return (data_len + 2) / 3 * 4;
}

size_t base64_encode_sz(size_t data_len)
{
    return base64_encode_min_sz(data_len) + 1;
}

const char *base64_encode(char *buf, size_t bufsz, const uint8_t *data, size_t data_len)
{
    if (!data || data_len <= 0)
    {
        return NULL;
    }
    auto enc_min_sz = base64_encode_min_sz(data_len);
    if (buf == NULL)
    {
        bufsz = enc_min_sz + 1;
        buf   = (char *)calloc(1, bufsz);
    }
    if (!buf || enc_min_sz > bufsz)
    {
        return NULL;
    }
    size_t group_cnt = data_len / 3;
    size_t rem_byte  = data_len % 3;

    auto dst = buf;
    auto src = data;
    for (size_t i = 0; i < group_cnt; ++i, dst += 4, src += 3)
    {
        base64_encode_3bytes(dst, src);
    }
    if (rem_byte == 1)
    {
        base64_encode_1bytes(dst, src);
    } else if (rem_byte == 2)
    {
        base64_encode_2bytes(dst, src);
    }

    // no space left, return
    if (enc_min_sz == bufsz)
    {
        return buf;
    }
    // pad NULL teminate
    buf[enc_min_sz] = '\0';
    return buf;
}

static size_t base64_decode_min_sz(size_t str_len)
{
    return str_len / 4 * 3;
}

size_t base64_decode_sz(size_t str_len)
{
    return base64_decode_min_sz(str_len) + 1;
}

const uint8_t *base64_decode(uint8_t *buf, size_t bufsz, const char *b64, size_t max_dec_len, size_t *dec_len)
{
    b64_dec_table_init();
    if (!b64)
    {
        return NULL;
    }
    auto len = strlen(b64);
    if (len <= 0)
    {
        return NULL;
    }
    if (max_dec_len && len > max_dec_len)
    {
        len = max_dec_len;
    }
    if (buf == NULL)
    {
        bufsz = base64_decode_sz(len);
        buf   = (uint8_t *)calloc(1, bufsz);
    }
    if (!buf)
    {
        return NULL;
    }
    //
    auto group_cnt = len / 4;
    auto rem_cnt   = len % 4;  // should be zero
    //
    if (rem_cnt)
    {
        // return NULL;
    }

    auto src    = b64;
    auto dst    = buf;
    auto dst_sz = bufsz;
    for (size_t i = 0; i < group_cnt && dst_sz >= 3; ++i, src += 4, dst += 3, dst_sz -= 3)
    {
        base64_decode_4char(dst, src);
    }
    if (dst_sz)
    {
        dst[0] = '\0';
    }

    auto dc = dst - buf;
    if (dc > 0)
    {
        if (*(src - 1) == b64_padchar)
        {
            --dc;
        }
        if (*(src - 2) == b64_padchar)
        {
            --dc;
        }
    }
    if (dec_len)
    {
        *dec_len = dc;
    }

    return buf;
}
