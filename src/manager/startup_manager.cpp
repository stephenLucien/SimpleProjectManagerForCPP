#include "startup_manager..h"

//
#include <string>
#include <unordered_map>
#include "utils/os_tools.h"
#include "cpp_helper/cpphelper_pthread.hpp"

namespace
{
class StartupInstance
{
   public:
    StartupInstance(StartupFunc func = NULL, void* data = NULL) : func(func), data(data)
    {
    }
    int run(int reason)
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
        OS_LOGV("startup %p: ret=%d, timelapse_ms=%llu", func, ret, end_ts - begin_ts);
        return ret;
    }
    //
    StartupFunc func;
    //
    void* data;
};

class StartupManager
{
   private:
    StartupManager()
    {
    }
    ~StartupManager()
    {
    }
    PthreadMutex m_mtx;
    //
    std::unordered_map<std::string, StartupInstance> m_starts;

   public:
    static StartupManager* getInstance()
    {
        static StartupManager obj;
        return &obj;
    }

    int reg(const char* tag, StartupFunc func, void* data)
    {
        if (!tag || !func)
        {
            return -1;
        }
        PthreadMutex::Writelock _l(m_mtx);
        //
        m_starts[std::string(tag)] = StartupInstance(func, data);
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
        auto itr = m_starts.find(tag);
        if (itr == m_starts.end())
        {
            return 0;
        }
        m_starts.erase(itr);
        return 1;
    }

    int run(const char* tag, int reason)
    {
        //
        PthreadMutex::Writelock _l(m_mtx);
        //
        if (tag)
        {
            auto itr = m_starts.find(tag);
            if (itr == m_starts.end())
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
            for (auto& obj : m_starts)
            {
                OS_LOGV("int '%s'(%p)(int %d, void* %p)", obj.first.c_str(), obj.second.func, reason, obj.second.data);
                obj.second.run(reason);
            }
        }
        return 0;
    }
};

}  // namespace


int reg_startup(const char* tag, StartupFunc func, void* data)
{
    return StartupManager::getInstance()->reg(tag, func, data);
}

int unreg_startup(const char* tag)
{
    return StartupManager::getInstance()->unreg(tag);
}

int run_startup(const char* tag, int reason)
{
    return StartupManager::getInstance()->run(tag, reason);
}

void os_init_on_startup_impl(int sig_num)
{
    OS_LOGV("%s", __FILE__);
    run_startup(NULL, sig_num);
}
