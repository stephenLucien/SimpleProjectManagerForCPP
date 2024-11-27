#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "app/app.h"
#include "manager/test_manager.h"
#include "utils/os_tools.h"

int main(int argc, char *argv[])
{
    os_setup_backtrace();
    os_setup_exit();
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
    // run_test("netstatus_manager");
    // run_test("tcp_ping4");
    // run_test("tcp_ping6");
    // run_test("tcp_wan6");
    // sync();
    // run_test("icmp_ping4");
    // run_test("icmp_ping6");
    // sync();
    // run_test("icmp_wan6");
    // run_test("unordered_multimap");
    // run_test("ordered_multimap");
    // usleep(1000 * 1000 * 10);

    // run_test("text_file_write");
    // run_test("text_file_read");

    // run_test("ifaces_dump");

    // run_test("map_itr_erase");
    run_test("list_itr_erase");


    return 0;
}
