#include <malloc.h>
#include <pthread.h>
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

        int state = PTHREAD_CANCEL_ENABLE;
        //
        state = PthreadWrapper::disable_cancel();
        OS_LOGW("state=%d", state);
        auto leak = malloc(16);
        OS_LOGW("leak: %p", leak);
        //
        sleep(1000 * 5);
        if (leak)
        {
            OS_LOGW("");
            free(leak);
        }
        OS_LOGW("");
        PthreadWrapper::setcancelstate(state);
        OS_LOGW("continue 1, %d", exitPending());
        pthread_testcancel();
        OS_LOGW("continue 2, %d", exitPending());

        return true;
    }

   public:
    TestPthread()
    {
        setNormalExitTimeout(3000);
    }
    ~TestPthread()
    {
    }

    static TestPthread* getInstance()
    {
        static TestPthread obj;
        return &obj;
    }

    bool tryRun(const char* task_name = NULL)
    {
        if (isRunning())
        {
            return false;
        }
        return run(task_name);
    }
};
}  // namespace

static int test_pthread_class(int reason, void* userdata)
{
    // TestPthread::getInstance()->tryRun("static obj");
    // sleep(1);
    // TestPthread::getInstance()->cancel(true);
    // OS_LOGD("after cancel detach");
    // sleep(10);

    TestPthread* pobj = new TestPthread();
    pobj->tryRun("new obj");
    sleep(1);
    OS_LOGD("delete obj 1");
    delete pobj;
    OS_LOGD("delete obj 2");

    return 0;
}

REG_TEST_FUNC(test_pthread_class, test_pthread_class, NULL)
