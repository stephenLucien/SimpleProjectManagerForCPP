#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

static void print_help_msg()
{
    //
    OS_LOGD(
        "\n"
        "--test \"test1,test2,test3\" ");
    exit(0);
}

int main(int argc, char *argv[])
{
    //
    auto parse_cnt = run_cmdline_arg_parser(argc, argv);
    //
    if (parse_cnt <= 0)
    {
        print_help_msg();
    }
    //
    os_init_on_startup(0);
    //
    run_tests();
    //
    return os_running_loop();
}
