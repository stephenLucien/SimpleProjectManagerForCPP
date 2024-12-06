#ifndef __BASE64_CODEC_H__
#define __BASE64_CODEC_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_encode_sz(size_t data_len);

const char *base64_encode(char *buf, size_t bufsz, const uint8_t *data, size_t data_len);

size_t base64_decode_sz(size_t str_len);

const uint8_t *base64_decode(uint8_t *buf, size_t bufsz, const char *b64, size_t max_dec_len, size_t *dec_len);

#ifdef __cplusplus
}
#endif

#endif  // __BASE64_CODEC_H__
