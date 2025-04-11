
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>


//
#include "cpp_helper/cpphelper_inotify.hpp"
#include "cpp_helper/cpphelper_pthread.hpp"
#include "manager/buffer_manager.hpp"
#include "manager/cleanup_manager.h"
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

namespace
{
//
static std::string m_test_path = "/tmp/inotify_tester";
//
class InotifyTester : public PthreadWrapper
{
   private:
    bool readyToRun() override
    {
        return true;
    }
    bool threadLoop() override
    {
        msleep_sliced(5000);
        if (exitPending())
        {
            return false;
        }
        touch(m_test_path);

        return true;
    }

   public:
    InotifyTester()
    {
        setTaskName("InotifyTester");
    }
    static int touch(const std::string &path, size_t buffer_sz = 256)
    {
        int ret = -1;
        //
        if (buffer_sz <= 1)
        {
            return ret;
        }
        //
        std::vector<char> buffer_container(buffer_sz);
        //
        auto buf    = buffer_container.data();
        auto bufsz0 = buffer_container.size();
        auto bufsz  = bufsz0 - 1;

        memset(buf, 0, bufsz0);
        //
        OS_LOGV("touch %s", path.c_str());
        //
        ret = system_wrap(buf, bufsz, "echo %d > %s", rand(), path.c_str());

        return ret;
    }
};

static std::shared_ptr<InotifyTester> m_tester;
static std::shared_ptr<InotifyHelper> m_helper;
};  // namespace

static int inotify_helper_end(int num, void *data)
{
    if (!((m_tester && m_tester->isRunning()) || (m_helper && m_helper->isRunning())))
    {
        return 0;
    }
    //
    OS_LOGD("inotify_helper test end0.");
    if (m_tester)
    {
        m_tester->requestExitAndWait(1000);
    }
    if (m_helper)
    {
        m_helper->requestExitAndWait(2000);
    }
    //
    OS_LOGD("inotify_helpertest end1.");
    return 0;
}
REG_CLEANUP_FUNC(inotify_helper_end, inotify_helper_end, NULL)

static int test_inotify_helper(int reason, void *userdata)
{
    //
    OS_LOGD("inotify_helper test begin.");
    //
    InotifyTester::touch(m_test_path);
    //
    m_helper = std::make_shared<InotifyHelper>();
    //
    if (m_helper)
    {
        m_helper->addWatch(m_test_path);
        //
        m_helper->tryRun("InotifyHelper");
    }
    m_tester = std::make_shared<InotifyTester>();
    //
    if (m_tester)
    {
        m_tester->tryRun();
    }

    return 0;
}

REG_TEST_FUNC(test_inotify_helper, test_inotify_helper, NULL)
