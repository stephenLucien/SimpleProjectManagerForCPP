#include <cstring>
#include <iostream>

#include "app/app.h"
#include "utils/compile_time.h"
#include "utils/curl_wrapper.hpp"
#include "utils/os_tools.h"

#include "test/test.h"

int main(int argc, char *argv[])
{
    os_setup_backtrace();
    //
    OS_PRINT(
        "%s:\n"
        "\tThis app built at %s",
        argv[0],
        COMPILE_TIME_STR(app));
    //
    run_test("buffer_manager");
    // run_test("curl_wrapper_get_baidu");

    return 0;
}
