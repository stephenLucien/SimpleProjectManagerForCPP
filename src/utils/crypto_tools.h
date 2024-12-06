#ifndef __CRYPTO_TOOLS_H__
#define __CRYPTO_TOOLS_H__


#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


int openssl_base64Decode(const char *b64message, uint8_t **data, size_t *datalen);

/**
 * @brief
 *
 * @param data
 * @param data_len
 * @param base64str :
 * - Character Set: Base64 uses a set of 64 characters: A-Z, a-z, 0-9, +, and /
 * - Groups of 6 Bits: The binary data is divided into groups of 6 bits because 2^6 = 64,
 * so the data perfectly fitting into the set of 64 characters. 4 * 6 bits = 3 * 8 bits = 3 bytes
 * - Padding: It may include one or two = padding characters at the end of the encoded string
 * to ensure the output length is a multiple of 4
 * - example:
 * -- data_len % 3 == 0 : no padding
 * -- data_len % 3 == 1 :
 * -- data_len % 3 == 2 :
 * @return int
 */
int openssl_base64Encode(const uint8_t *data, size_t data_len, char **base64str);

int openssl_md5(const uint8_t *data, size_t datalen, unsigned char md[64], size_t *mdlen);

int openssl_hmacmd5(const uint8_t *data, size_t datalen, const uint8_t *key, size_t keylen, uint8_t mac[64], size_t *maclen);

int openssl_hmacsha1(const uint8_t *data, size_t datalen, const uint8_t *key, size_t keylen, uint8_t mac[64], size_t *maclen);

int openssl_hmacsha256(const uint8_t *data, size_t datalen, const uint8_t *key, size_t keylen, uint8_t mac[64], size_t *maclen);

int openssl_cipher_encode_aes_cbc128(const uint8_t *key,
                                     size_t         key_bitlen,
                                     const uint8_t *init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t *input,
                                     size_t         inputlen,
                                     uint8_t      **output,
                                     size_t        *outputlen);

int openssl_cipher_decode_aes_cbc128(const uint8_t *key,
                                     size_t         key_bitlen,
                                     const uint8_t *init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t *input,
                                     size_t         inputlen,
                                     uint8_t      **output,
                                     size_t        *outputlen);

int openssl_cipher_encode_aes_ecb128(const uint8_t *key,
                                     size_t         key_bitlen,
                                     const uint8_t *init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t *input,
                                     size_t         inputlen,
                                     uint8_t      **output,
                                     size_t        *outputlen);

int openssl_cipher_decode_aes_ecb128(const uint8_t *key,
                                     size_t         key_bitlen,
                                     const uint8_t *init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t *input,
                                     size_t         inputlen,
                                     uint8_t      **output,
                                     size_t        *outputlen);


int openssl_cipher_encode_aes_cbc128_nopadding(const uint8_t *key,
                                               size_t         key_bitlen,
                                               const uint8_t *init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t *input,
                                               size_t         inputlen,
                                               uint8_t      **output,
                                               size_t        *outputlen);

int openssl_cipher_decode_aes_cbc128_nopadding(const uint8_t *key,
                                               size_t         key_bitlen,
                                               const uint8_t *init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t *input,
                                               size_t         inputlen,
                                               uint8_t      **output,
                                               size_t        *outputlen);

int openssl_cipher_encode_aes_ecb128_nopadding(const uint8_t *key,
                                               size_t         key_bitlen,
                                               const uint8_t *init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t *input,
                                               size_t         inputlen,
                                               uint8_t      **output,
                                               size_t        *outputlen);

int openssl_cipher_decode_aes_ecb128_nopadding(const uint8_t *key,
                                               size_t         key_bitlen,
                                               const uint8_t *init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t *input,
                                               size_t         inputlen,
                                               uint8_t      **output,
                                               size_t        *outputlen);

int openssl_rsa_sign_sha1(const uint8_t *pri_key, size_t pri_key_len, const uint8_t *data, size_t datalen, uint8_t sig[1024], size_t *siglen);

int openssl_rsa_sign_sha256(const uint8_t *pri_key, size_t pri_key_len, const uint8_t *data, size_t datalen, uint8_t sig[1024], size_t *siglen);

#ifdef __cplusplus
}
#endif


#endif  // __CRYPTO_TOOLS_H__
