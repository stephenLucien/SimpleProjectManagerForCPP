#include <malloc.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
//
#include "cpp_helper/cpphelper_debug.hpp"
#include "cpp_helper/cpphelper_os.hpp"
#include "manager/test_manager.h"
#include "map-macro/map.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"
#include "utils/pthread_class.hpp"

namespace
{
class TestPthread : public PthreadWrapper
{
   private:
    bool readyToRun()
    {
        return true;
    }
    bool threadLoop()
    {
        sleep(500);
        if (exitPending())
        {
            OS_LOGD("");
            return false;
        }

        PTHREAD_MALLOC(char, ptr, 4);
        OS_LOGI("%p", ptr);
        //
        sleep(1000 * 30);

        return true;
    }


    TestPthread()
    {
    }
    ~TestPthread()
    {
    }

   public:
    static TestPthread* getInstance()
    {
        static TestPthread obj;
        return &obj;
    }
    bool tryRun(const std::string& taskName = "TestPthread")
    {
        if (isRunning())
        {
            return false;
        }
        return run(taskName.c_str());
    }
};
}  // namespace

static int test_pthread_class(int reason, void* userdata)
{
    TestPthread::getInstance()->tryRun();
    TestPthread::getInstance()->wait_loop(2000);
    TestPthread::getInstance()->cancel();
    return 0;
}

REG_TEST_FUNC(test_pthread_class, test_pthread_class, NULL)
