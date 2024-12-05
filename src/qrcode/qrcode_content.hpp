#ifndef __QRCODE_CONTENT_H__
#define __QRCODE_CONTENT_H__

#include <string>

/**
 * @brief
 * - WEP: Wired Equivalent Privacy (less secure, older standard).
 * - WPA: Wi-Fi Protected Access.
 * - WPA2: Wi-Fi Protected Access II (more secure, commonly used).
 * - nopass: If your Wi-Fi network does not use any encryption (open network).
 *
 */
typedef enum
{
    QRCODE_WIFI_ENCRYPT_NONE,
    QRCODE_WIFI_ENCRYPT_WEP,
    QRCODE_WIFI_ENCRYPT_WPA,
    QRCODE_WIFI_ENCRYPT_WPA2
} EnQrCodeWifiEncryptFmt;

/**
 * @brief generate the content of QrCode，
 * so the Android or ios device can connect to this WiFi by scanning the QrCode
 *
 * @param outstring
 * - "WIFI:S:YourSSID;T:WPA2;P:YourPassword;;"
 * @param SSID
 * @param PSK
 * @param fmt
 * @return int
 */
int gen_qrcode_wifi_content(std::string           &outstring,
                            const std::string     &SSID,
                            const std::string     &PSK,
                            EnQrCodeWifiEncryptFmt fmt    = QRCODE_WIFI_ENCRYPT_WPA2,
                            bool                   hidden = false);

#endif  // __QRCODE_CONTENT_H__
