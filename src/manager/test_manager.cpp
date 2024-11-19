#include "test_manager.h"

//
#include <string>
#include <unordered_map>
#include "utils/os_tools.h"
#include "utils/pthread_mutex.hpp"

namespace
{
class TestInstance
{
   private:
    //
    TestFunc func;
    //
    void* data;

   public:
    TestInstance(TestFunc func = NULL, void* data = NULL) : func(func), data(data)
    {
    }
    int run()
    {
        int ret = -1;
        //
        if (!func)
        {
            return ret;
        }

        auto begin_ts = os_get_timestamp_ms();
        ret           = func(data);
        auto end_ts   = os_get_timestamp_ms();
        OS_LOGD("test %p: ret=%d, timelapse_ms=%llu", func, ret, end_ts - begin_ts);
        return ret;
    }
};

class TestManager
{
   private:
    TestManager()
    {
    }
    ~TestManager()
    {
    }
    PthreadMutex m_mtx;
    //
    std::unordered_map<std::string, TestInstance> m_tests;

   public:
    static TestManager* getInstance()
    {
        static TestManager obj;
        return &obj;
    }

    int reg(const char* tag, TestFunc func, void* data)
    {
        if (!tag || !func)
        {
            return -1;
        }
        PthreadMutex::Writelock _l(m_mtx);
        m_tests[std::string(tag)] = TestInstance(func, data);
        return 0;
    }

    int unreg(const char* tag)
    {
        if (!tag)
        {
            return -1;
        }
        PthreadMutex::Writelock _l(m_mtx);
        auto                    itr = m_tests.find(tag);
        if (itr == m_tests.end())
        {
            return 0;
        }
        m_tests.erase(itr);
        return 1;
    }

    int run(const char* tag)
    {
        //
        PthreadMutex::Writelock _l(m_mtx);
        //
        if (tag)
        {
            auto itr = m_tests.find(tag);
            if (itr == m_tests.end())
            {
                return -1;
            } else
            {
                return itr->second.run();
            }
        } else
        {
            for (auto& obj : m_tests)
            {
                obj.second.run();
            }
        }
        return 0;
    }
};

}  // namespace


int reg_test(const char* tag, TestFunc func, void* data)
{
    return TestManager::getInstance()->reg(tag, func, data);
}

int unreg_test(const char* tag)
{
    return TestManager::getInstance()->unreg(tag);
}

int run_test(const char* tag)
{
    return TestManager::getInstance()->run(tag);
}
