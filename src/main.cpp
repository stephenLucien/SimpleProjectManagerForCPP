#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "app/app.h"
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_system.h"

int main(int argc, char *argv[])
{
    os_init_on_startup(0);

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
    // run_test("list_itr_erase");

    // run_test("test_opencv2");

    // run_test("test_base64");
    // run_test("test_base64_openssl");

    // run_test("test_printf");
    // run_test("tvs_songlist");
    // run_test("tvs_grouplist");
    // run_test("tvs_authorized_parse");
    // run_test("test_tvs_songGroups");
    // run_test("test_tvs_songTops");
    // run_test("test_tvs_songCollection");


    // run_test("qr_wifi_test");


    return os_running_loop();
}
