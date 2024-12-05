#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/test_manager.h"
#include "qrcode/qrcode.h"
#include "qrcode/qrcode_content.hpp"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"



int qr_wifi_test(int reason, void* userdata)
{
    std::string outstring;
    std::string SSID = "qwer";
    std::string PSK  = "12345678";
    //
    gen_qrcode_wifi_content(outstring, SSID, PSK);
    //
    dump_qrcode_to_ansi(outstring.c_str(), NULL);

    return 0;
}


REG_TEST_FUNC(qr_wifi_test, qr_wifi_test, NULL)
