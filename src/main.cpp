#include <unistd.h>
#include <cstring>
#include <iostream>

#include "app/app.h"
#include "utils/compile_time.h"
#include "utils/curl_wrapper.hpp"
#include "utils/os_tools.h"

#include "manager/test_manager.h"

int main(int argc, char *argv[])
{
    os_setup_backtrace();
    //
    OS_PRINT(
        "%s:\n"
        "\tThis app built at %s",
        argv[0],
        COMPILE_TIME_STR(app));

    if (is_sudo(1))
    {
        OS_PRINT("Run as sudoer!!!");
    }
    //
    // run_test("buffer_manager");
    // run_test("curl_wrapper_get_baidu");
    // run_test("os_tools_net");
    run_test("netstatus_manager");
    usleep(1000 * 1000 * 10);

    return 0;
}
