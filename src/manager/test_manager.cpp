#include "test_manager.h"

//
#include <string>
#include <unordered_map>
#include "utils/os_tools.h"
#include "utils/pthread_class.hpp"

namespace
{
class TestInstance
{
   public:
    TestInstance(TestFunc func = NULL, void* data = NULL) : func(func), data(data)
    {
    }
    int run(int reason = 0)
    {
        int ret = -1;
        //
        if (!func)
        {
            return ret;
        }

        auto begin_ts = os_get_timestamp_ms();
        //
        ret = func(reason, data);
        //
        auto end_ts = os_get_timestamp_ms();
        OS_LOGV("test %p: ret=%d, timelapse_ms=%llu", func, ret, end_ts - begin_ts);
        return ret;
    }
    //
    TestFunc func;
    //
    void* data;
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
        //
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
        //
        auto itr = m_tests.find(tag);
        if (itr == m_tests.end())
        {
            return 0;
        }
        m_tests.erase(itr);
        return 1;
    }

    int run(const char* tag, int reason = 0)
    {
        //
        PthreadMutex::Writelock _l(m_mtx);
        //
        if (tag)
        {
            auto itr = m_tests.find(tag);
            if (itr == m_tests.end())
            {
                OS_LOGW("<%s> not found: %s", __PRETTY_FUNCTION__, tag);
                return -1;
            } else
            {
                OS_LOGV("int '%s'(%p)(int %d, void* %p)", itr->first.c_str(), itr->second.func, reason, itr->second.data);
                return itr->second.run(reason);
            }
        } else
        {
            for (auto& obj : m_tests)
            {
                OS_LOGV("int '%s'(%p)(int %d, void* %p)", obj.first.c_str(), obj.second.func, reason, obj.second.data);
                obj.second.run(reason);
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

int run_test(const char* tag, int reason)
{
    return TestManager::getInstance()->run(tag, reason);
}
