#include "os_tools.h"

//
#include <unistd.h>


int is_sudo(int dump)
{
    auto uid  = getuid();
    auto euid = geteuid();

    if (uid == 0)
    {
        if (dump)
        {
            OS_LOGV("root");
        }
        return 1;
    }
    if (euid == 0)
    {
        if (dump)
        {
            OS_LOGV("sudo");
        }
        return 1;
    }
    if (dump)
    {
        OS_LOGV("uid: %u", uid);
        OS_LOGV("euid: %u", euid);
    }
    return 0;
}
