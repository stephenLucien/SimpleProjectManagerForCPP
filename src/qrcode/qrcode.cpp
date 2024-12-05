#include "qrcode.h"

#include <string>

#include "qrencode.h"

#include "utils/ansi_color.h"
#include "utils/os_tools_log.h"

void dump_qrcode_to_ansi(const QRcode *qrcode, const char *square_str = NULL)
{
    if (!qrcode)
    {
        return;
    }
    // std::string black = std::string(
    //     "\xE2\x96\x88"
    //     "\xE2\x96\x88");

    std::string to_black = ANSI_BACKGROUND_BLACK;
    std::string to_white = ANSI_BACKGROUND_WHITE;
    std::string square   = square_str ? square_str : "  ";
    std::string reset    = ANSI_RESET;

    //
    std::string dumpstr;
    //
    unsigned char last_flag;
    for (int r = 0; r < qrcode->width; ++r)
    {
        dumpstr   += "\n";
        last_flag  = 0xFF;
        for (int c = 0; c < qrcode->width; ++c)
        {
            auto pixel = qrcode->data[r * qrcode->width + c];
            //
            auto cur_flag = (pixel & 0x01);
            if (cur_flag != last_flag)
            {
                last_flag = cur_flag;
                //
                if (cur_flag)
                {
                    dumpstr += to_black;
                } else
                {
                    dumpstr += to_white;
                }
            }
            dumpstr += square;
        }
        dumpstr += reset;
    }
    //
    OS_LOGD("(len=%zu):", dumpstr.length());
    os_log_write_impl(OS_LOG_INFO, "qrcode", dumpstr.c_str(), dumpstr.length());
}

void dump_qrcode_to_ansi(const char *content, const char *square_str)
{
    if (!content || strlen(content) <= 0)
    {
        return;
    }
    OS_LOGD("qrcode content:\n%s", content);
    //
    auto qrcode = QRcode_encodeString(content, 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    if (qrcode)
    {
        dump_qrcode_to_ansi(qrcode);
        QRcode_free(qrcode);
    }
}
