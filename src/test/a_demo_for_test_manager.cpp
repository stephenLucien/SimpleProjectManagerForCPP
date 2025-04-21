#include <cstdint>
#include <iostream>
#include <map>
#include <string>
//
#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
//
#include "utils/os_tools_log.h"

#define ES(tag, code)                                 \
    static int test_##tag(int reason, void* userdata) \
    {                                                 \
        OS_LOGD("");                                  \
        {code};                                       \
        return 0;                                     \
    }                                                 \
    REG_TEST_FUNC(test_##tag, test_##tag, NULL)

ES(tag,
   /* test code block BEGIN */
   {
       //
   } /* test code block END */

)
