#include <unistd.h>
#include "app.h"

//
#include "utils/os_tools.h"

#include "manager/cleanup_manager.h"
#include "manager/startup_manager..h"

static int m_console_running = 1;

static int console_initialize(int num, void* data)
{
    //
    if (is_sudo(1))
    {
        OS_PRINT("Run as sudoer!!!");
    }
    //
    OS_PRINT("This app built at %s", COMPILE_TIME_STR(app));
    //
    return 0;
}
REG_STARTUP_FUNC(console_initialize, console_initialize, NULL)

static int console_deinitialize(int num, void* data)
{
    return 0;
}
REG_CLEANUP_FUNC(console_deinitialize, console_deinitialize, NULL)


//
void os_running_loop_impl()
{
    OS_LOGV("%s: Begin", __FILE__);
    while (m_console_running)
    {
        sleep(1);
    }
    OS_LOGV("%s: End", __FILE__);
}

void os_stop_running_loop_impl(int code)
{
    OS_LOGV("%s", __FILE__);
    m_console_running = 0;
}
