#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <vector>

#include "app/app.h"
#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
#include "utils/cstring_proc.h"
#include "utils/os_tools.h"
#include "utils/os_tools_system.h"

static void print_help_msg()
{
    //
    OS_LOGD(
        "\n"
        "--test \"test1 test2 test3\" ");
    exit(0);
}

REG_CMDLINE_ARG_PARSE_FUNC(0, 'h', "help", 0, {
    //
    print_help_msg();
})

static std::vector<std::string> tests_name;
REG_CMDLINE_ARG_PARSE_FUNC(1, 't', "test", 1, {
    //
    if (_param)
    {
        tests_name = str_split(_param, " ");
    }
})

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
    for (auto &t : tests_name)
    {
        run_test(t.c_str());
    }
    //
    return os_running_loop();
}
