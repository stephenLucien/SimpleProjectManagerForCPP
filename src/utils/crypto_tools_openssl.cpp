#include "crypto_tools.h"

//
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>

//
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include "openssl/err.h"



//
#include "common.h"
#include "os_tools.h"


int openssl_base64Decode(const char* b64message, uint8_t** data, size_t* datalen)
{
    if (!b64message)
    {
        return -1;
    }
    if (!data)
    {
        return -1;
    }
    size_t b64msglen = strlen(b64message);
    // Decodes a base64 encoded string
    BIO *bio, *b64;
    //
    int decodeLen = (b64msglen + 3) / 4 * 3 + 1, len = 0;
    *data = (uint8_t*)malloc(decodeLen);
    if (*data == NULL)
    {
        return -1;
    }
    memset(*data, 0, decodeLen);
    FILE* stream = fmemopen((void*)b64message, b64msglen, "r");
    if (stream == NULL)
    {
        FREE(*data);
        return -1;
    }

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stream, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);  // Do not use newlines to flush buffer
    len = BIO_read(bio, *data, b64msglen);

    if (datalen)
    {
        *datalen = len;
    }

    BIO_free_all(bio);
    fclose(stream);

    return (0);  // success
}

int openssl_base64Encode(const uint8_t* data, size_t data_len, char** base64str)
{
    if (!data || data_len <= 0)
    {
        return -1;
    }
    if (!base64str)
    {
        return -1;
    }
    // Encodes a string to base64
    BIO * bio, *b64;
    FILE* stream = NULL;
    //
    int encodedSize = (data_len + 2) / 3 * 4 + 1;
    //
    *base64str = (char*)malloc(encodedSize);
    if (*base64str == NULL)
    {
        return -1;
    }
    memset(*base64str, 0, encodedSize);

    stream = fmemopen(*base64str, encodedSize, "w");
    if (stream == NULL)
    {
        FREE(*base64str);
        return -1;
    }
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stream, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);  // Ignore newlines - write
                                                 // everything in one line
    BIO_write(bio, data, data_len);
    BIO_flush(bio);
    BIO_free_all(bio);
    fclose(stream);

    return (0);  // success
}

int openssl_md5(const uint8_t* data, size_t datalen, unsigned char md[64], size_t* mdlen)
{
    int ret = -1;
    //
    unsigned char* md5 = MD5(data, datalen, md);
    if (md5)
    {
        ret = 0;
        if (mdlen)
        {
            *mdlen = 16;
        }
    }
    return ret;
}

int openssl_hmac(const EVP_MD* md_info, const uint8_t* data, size_t datalen, const uint8_t* key, size_t keylen, uint8_t mac[64], size_t* maclen)
{
    int          ret = -1;
    unsigned int _maclen;
    // EVP_MAX_MD_SIZE
    uint8_t* md = HMAC(md_info, key, keylen, data, datalen, mac, &_maclen);
    if (md)
    {
        ret = 0;
        if (maclen)
        {
            *maclen = _maclen;
        }
    } else
    {
        os_log_printf(OS_LOG_ERR, "crypto", "HMAC fail, %s", ERR_reason_error_string(ERR_get_error()));
    }

    return ret;
}

int openssl_hmacmd5(const uint8_t* data, size_t datalen, const uint8_t* key, size_t keylen, uint8_t mac[64], size_t* maclen)
{
    //
    return openssl_hmac(EVP_md5(), data, datalen, key, keylen, mac, maclen);
}

int openssl_hmacsha1(const uint8_t* data, size_t datalen, const uint8_t* key, size_t keylen, uint8_t mac[64], size_t* maclen)
{
    //
    return openssl_hmac(EVP_sha1(), data, datalen, key, keylen, mac, maclen);
}

int openssl_hmacsha256(const uint8_t* data, size_t datalen, const uint8_t* key, size_t keylen, uint8_t mac[64], size_t* maclen)
{
    //
    return openssl_hmac(EVP_sha256(), data, datalen, key, keylen, mac, maclen);
}

/**
 * @brief
 *
 * @param cipher
 * @param b_encrypt
 * @param key
 * @param key_bitlen
 * @param init_iv
 * @param init_iv_len
 * @param input
 * @param inputlen
 * @param output
 * @param outputlen
 * @param padding : PKCS7 or nopadding
 * @return int32_t
 */
int32_t openssl_cipher_do_crypt(const EVP_CIPHER* cipher,
                                int               b_encrypt,
                                const uint8_t*    key,
                                size_t            key_bitlen,
                                const uint8_t*    init_iv,
                                size_t            init_iv_len,
                                const uint8_t*    input,
                                size_t            inputlen,
                                uint8_t**         output,
                                size_t*           outputlen,
                                uint8_t           b_padding = 1)
{
    int ret = -1;

    uint8_t* cipherText = NULL;
    size_t   cipherLen  = 0;

    if (!key)
    {
        return ret;
    }

    if (!input || inputlen <= 0)
    {
        return ret;
    }

    if (!output || !outputlen)
    {
        return ret;
    }

    EVP_CIPHER_CTX* ctx = NULL;
    do
    {
        ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_CIPHER_CTX_new fail, %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        ret = EVP_CipherInit_ex(ctx, cipher, NULL, key, init_iv, b_encrypt);
        if (ret != 1)
        {
            ret = -1;
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_CipherInit_ex fail, %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        if (!b_padding)
        {
            // default PKCS7 padding
            EVP_CIPHER_CTX_set_padding(ctx, 0);
        }

        auto   blocksize      = EVP_CIPHER_block_size(cipher);
        size_t inputblock     = (inputlen + blocksize - 1) / blocksize;
        size_t inputlen_align = inputblock * blocksize;
        size_t alloc_sz       = 2 * blocksize + inputlen_align;
        //
        cipherLen = alloc_sz;

        if (cipherLen <= 0)
        {
            ret = -1;
            break;
        }

        cipherText  = (uint8_t*)malloc(cipherLen);
        cipherLen  -= blocksize;
        if (!cipherText)
        {
            cipherLen = 0;
            ret       = -1;
            os_log_printf(OS_LOG_ERR, "crypto", "malloc: ret=%d", ret);
            break;
        }

        int tmplen = cipherLen;
        ret        = EVP_CipherUpdate(ctx, cipherText, &tmplen, input, inputlen);
        cipherLen  = tmplen;
        if (ret != 1)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_CipherUpdate fail, %s", ERR_reason_error_string(ERR_get_error()));
            ret = -1;
            break;
        }

        //
        int finalLen = 0;
        ret          = EVP_CipherFinal_ex(ctx, cipherText + cipherLen, &finalLen);
        if (ret != 1)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_CipherFinal_ex fail, %s", ERR_reason_error_string(ERR_get_error()));
            ret = -1;
            break;
        }
        cipherLen += finalLen;
        if (cipherLen > 0 && cipherLen <= alloc_sz)
        {
            cipherText[cipherLen] = 0;
            ret                   = 0;
        }
    } while (0);

    if (ret)
    {
        if (cipherText)
        {
            free(cipherText);
        }
    } else
    {
        *output    = cipherText;
        *outputlen = cipherLen;
    }

    EVP_CIPHER_CTX_free(ctx);

    return ret;
}

int openssl_cipher_encode_aes_cbc128(const uint8_t* key,
                                     size_t         key_bitlen,
                                     const uint8_t* init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t* input,
                                     size_t         inputlen,
                                     uint8_t**      output,
                                     size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_cbc();

    return openssl_cipher_do_crypt(cipher_type, 1, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 1);
}


int openssl_cipher_encode_aes_cbc128_nopadding(const uint8_t* key,
                                               size_t         key_bitlen,
                                               const uint8_t* init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t* input,
                                               size_t         inputlen,
                                               uint8_t**      output,
                                               size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_cbc();

    return openssl_cipher_do_crypt(cipher_type, 1, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 0);
}

int openssl_cipher_decode_aes_cbc128(const uint8_t* key,
                                     size_t         key_bitlen,
                                     const uint8_t* init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t* input,
                                     size_t         inputlen,
                                     uint8_t**      output,
                                     size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_cbc();

    return openssl_cipher_do_crypt(cipher_type, 0, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 1);
}


int openssl_cipher_decode_aes_cbc128_nopadding(const uint8_t* key,
                                               size_t         key_bitlen,
                                               const uint8_t* init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t* input,
                                               size_t         inputlen,
                                               uint8_t**      output,
                                               size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_cbc();

    return openssl_cipher_do_crypt(cipher_type, 0, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 0);
}

int openssl_cipher_encode_aes_ecb128(const uint8_t* key,
                                     size_t         key_bitlen,
                                     const uint8_t* init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t* input,
                                     size_t         inputlen,
                                     uint8_t**      output,
                                     size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_ecb();

    return openssl_cipher_do_crypt(cipher_type, 1, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 1);
}


int openssl_cipher_encode_aes_ecb128_nopadding(const uint8_t* key,
                                               size_t         key_bitlen,
                                               const uint8_t* init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t* input,
                                               size_t         inputlen,
                                               uint8_t**      output,
                                               size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_ecb();

    return openssl_cipher_do_crypt(cipher_type, 1, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 0);
}

int openssl_cipher_decode_aes_ecb128(const uint8_t* key,
                                     size_t         key_bitlen,
                                     const uint8_t* init_iv,
                                     size_t         init_iv_len,
                                     const uint8_t* input,
                                     size_t         inputlen,
                                     uint8_t**      output,
                                     size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_ecb();

    return openssl_cipher_do_crypt(cipher_type, 0, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 1);
}


int openssl_cipher_decode_aes_ecb128_nopadding(const uint8_t* key,
                                               size_t         key_bitlen,
                                               const uint8_t* init_iv,
                                               size_t         init_iv_len,
                                               const uint8_t* input,
                                               size_t         inputlen,
                                               uint8_t**      output,
                                               size_t*        outputlen)
{
    //
    auto cipher_type = EVP_aes_128_ecb();

    return openssl_cipher_do_crypt(cipher_type, 0, key, key_bitlen, init_iv, init_iv_len, input, inputlen, output, outputlen, 0);
}


RSA* openssl_create_RSA(int is_pub, const char* key)
{
    RSA* rsa    = NULL;
    BIO* keybio = NULL;
    do
    {
        keybio = BIO_new_mem_buf((void*)key, -1);
        if (!keybio)
        {
            break;
            ;
        }
        if (is_pub)
        {
            rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
        } else
        {
            rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
        }
        if (!rsa)
        {
            break;
        }
    } while (0);

    if (keybio)
    {
        BIO_free_all(keybio);
    }
    return rsa;
}

void openssl_delete_RSA(RSA* rsa)
{
    RSA_free(rsa);
}

EVP_PKEY* openssl_create_pkey(int is_pub, const char* key)
{
    EVP_PKEY* pkey = NULL;

    RSA* rsa = openssl_create_RSA(is_pub, key);
    if (!rsa)
    {
        return pkey;
    }
    pkey = EVP_PKEY_new();
    if (!pkey)
    {
        return pkey;
    }
    if (EVP_PKEY_assign_RSA(pkey, rsa) <= 0)
    {
        os_log_printf(OS_LOG_ERR, "crypto", "EVP_PKEY_assign_RSA fail, %s", ERR_reason_error_string(ERR_get_error()));
        EVP_PKEY_free(pkey);
        return NULL;
    }
    return pkey;
}

void openssl_delete_pkey(EVP_PKEY* pkey)
{
    EVP_PKEY_free(pkey);
}

int openssl_rsa_sign(
    const EVP_MD* mdtype, const uint8_t* pri_key, size_t pri_key_len, const uint8_t* data, size_t datalen, uint8_t sig[1024], size_t* siglen)
{
    int ret = -1;

    EVP_MD_CTX* ctx  = NULL;
    EVP_PKEY*   pkey = NULL;
    do
    {
        pkey = openssl_create_pkey(0, (const char*)pri_key);
        if (!pkey)
        {
            break;
        }
        ctx = EVP_MD_CTX_create();
        if (!ctx)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_DigestSignInit fail, %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        if (EVP_DigestSignInit(ctx, NULL, mdtype, NULL, pkey) <= 0)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_DigestSignInit fail, %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        if (EVP_DigestSignUpdate(ctx, data, datalen) <= 0)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_DigestSignUpdate fail, %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        size_t sigsz = 1024;
        if (EVP_DigestSignFinal(ctx, sig, &sigsz) <= 0)
        {
            os_log_printf(OS_LOG_ERR, "crypto", "EVP_DigestSignFinal fail, %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        if (siglen)
        {
            *siglen = sigsz;
        }
        ret = 0;
    } while (0);


    if (ctx)
    {
        EVP_MD_CTX_free(ctx);
    }

    if (pkey)
    {
        openssl_delete_pkey(pkey);
    }

    return ret;
}

int openssl_rsa_sign_sha1(const uint8_t* pri_key, size_t pri_key_len, const uint8_t* data, size_t datalen, uint8_t sig[1024], size_t* siglen)
{
    return openssl_rsa_sign(EVP_sha1(), pri_key, pri_key_len, data, datalen, sig, siglen);
}

int openssl_rsa_sign_sha256(const uint8_t* pri_key, size_t pri_key_len, const uint8_t* data, size_t datalen, uint8_t sig[1024], size_t* siglen)
{
    return openssl_rsa_sign(EVP_sha256(), pri_key, pri_key_len, data, datalen, sig, siglen);
}
