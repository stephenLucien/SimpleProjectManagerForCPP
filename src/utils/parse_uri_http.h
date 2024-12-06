#ifndef __PARSE_URI_HTTP_H__
#define __PARSE_URI_HTTP_H__


#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
This function converts the given input string to a URL encoded string and returns that as a new allocated string. All input characters that are not
a-z, A-Z, 0-9, '-', '.', '_' or '~' are converted to their "URL escaped" version (%NN where NN is a two-digit hexadecimal number).
 *
 * @param raw
 * @param ignore
 * @param out
 * @param outsz
 * @return int
 */
int http_url_encode(const char *raw, size_t rawlen, const char *ignore, char *out, size_t outsz);


/**
 * @brief parse http or https URL
 *
 * @param [in] url :
 * https://111.10.38.128:8089/upload/file/2024/03/05/test.json
 * @param [out] btls
 * - set to 1 if url prefix with 'https://'
 * @param [out] host
 * - host name
 * @param [in] hostlen
 * - buffer size of host
 * @param [out] port
 * - port
 * @param [out] path
 * - resource path
 * @param [in] pathlen
 * - buffer size of path
 * @return int
 * - 0 : success
 * - other : fail
 */
int parse_uri_http(const char *url, uint8_t *btls, char *host, size_t hostlen, uint16_t *port, char *path, size_t pathlen);

#ifdef __cplusplus
}
#endif


#endif  // __PARSE_URI_HTTP_H__
