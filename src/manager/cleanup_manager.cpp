#include "cleanup_manager.h"



//
#include <string>
#include <unordered_map>
#include "utils/os_tools.h"
#include "utils/pthread_class.hpp"

namespace
{
class CleanupInstance
{
   public:
    CleanupInstance(CleanupFunc func = NULL, void* data = NULL) : func(func), data(data)
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
        OS_LOGV("cleanup %p: ret=%d, timelapse_ms=%llu", func, ret, end_ts - begin_ts);
        return ret;
    }
    //
    CleanupFunc func;
    //
    void* data;
};

class CleanupManager
{
   private:
    CleanupManager()
    {
    }
    ~CleanupManager()
    {
    }
    PthreadMutex m_mtx;
    //
    std::unordered_map<std::string, CleanupInstance> m_cleans;

   public:
    static CleanupManager* getInstance()
    {
        static CleanupManager obj;
        return &obj;
    }

    int reg(const char* tag, CleanupFunc func, void* data)
    {
        if (!tag || !func)
        {
            return -1;
        }
        PthreadMutex::Writelock _l(m_mtx);
        //
        m_cleans[std::string(tag)] = CleanupInstance(func, data);
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
        auto itr = m_cleans.find(tag);
        if (itr == m_cleans.end())
        {
            return 0;
        }
        m_cleans.erase(itr);
        return 1;
    }

    int run(const char* tag, int reason)
    {
        //
        PthreadMutex::Writelock _l(m_mtx);
        //
        if (tag)
        {
            auto itr = m_cleans.find(tag);
            if (itr == m_cleans.end())
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
            for (auto& obj : m_cleans)
            {
                OS_LOGV("int '%s'(%p)(int %d, void* %p)", obj.first.c_str(), obj.second.func, reason, obj.second.data);
                obj.second.run(reason);
            }
        }
        return 0;
    }
};

}  // namespace


int reg_cleanup(const char* tag, CleanupFunc func, void* data)
{
    return CleanupManager::getInstance()->reg(tag, func, data);
}

int unreg_cleanup(const char* tag)
{
    return CleanupManager::getInstance()->unreg(tag);
}

int run_cleanup(const char* tag, int reason)
{
    return CleanupManager::getInstance()->run(tag, reason);
}

void os_deinit_on_exit_impl(int sig_num)
{
    OS_LOGV("%s", __FILE__);
    run_cleanup(NULL, sig_num);
}
