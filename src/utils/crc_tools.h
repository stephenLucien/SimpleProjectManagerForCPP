#ifndef __CRC_TOOLS_H__
#define __CRC_TOOLS_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t calcrc16_ccitt_false(const uint8_t* data, size_t len);


#ifdef __cplusplus
}
#endif


#endif  // __CRC_TOOLS_H__
