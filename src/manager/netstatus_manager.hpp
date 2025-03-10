#ifndef __NETSTATUS_MANAGER_H__
#define __NETSTATUS_MANAGER_H__

//
#include "utils/os_tools_net.h"
#include "cpp_helper/cpphelper_pthread.hpp"

class NetStatusManager : public PthreadWrapper
{
   private:
    int idle;
    int interval_ms;
    int probes;
    //
    bool en4;
    int  left_idle4;
    int  good4;
    //
    bool en6;
    int  left_idle6;
    int  good6;

    NetStatusManager()
    {
        //
        idle = 7;
        //
        interval_ms = 1000;
        //
        probes = 5;
        //
        en4        = true;
        left_idle4 = 0;
        good4      = 0;
        //
        en6        = false;
        left_idle6 = 0;
        good6      = 0;
    }

    ~NetStatusManager()
    {
    }

    void check4()
    {
        if (left_idle4 > 0)
        {
            --left_idle4;
            return;
        }

        //
        auto ret = os_net_check_wan4();
        //
        if (ret == 0)
        {
            if (good4 == 0)
            {
                OS_LOGI("wan4 changed up!");
            }
            good4 = probes;
            //
            left_idle4 = idle;
        } else
        {
            OS_LOGV("check_wan4 fail");
            if (good4 > 0)
            {
                --good4;
                if (good4 == 0)
                {
                    OS_LOGI("wan4 changed down!");
                }
            }
        }
    }

    void check6()
    {
        if (left_idle6 > 0)
        {
            --left_idle6;
            return;
        }

        //
        auto ret = os_net_check_wan6();
        //
        if (ret == 0)
        {
            if (good6 == 0)
            {
                OS_LOGI("wan6 changed up!");
            }
            good6 = probes;
            //
            left_idle6 = idle;
        } else
        {
            OS_LOGV("check_wan6 fail");
            if (good6 > 0)
            {
                --good6;
                if (good4 == 0)
                {
                    OS_LOGI("wan6 changed down!");
                }
            }
        }
    }

    bool threadLoop()
    {
        if (exitPending())
        {
            return false;
        }
        usleep(1000 * interval_ms);

        if (en4)
        {
            check4();
        }

        if (en6)
        {
            check6();
        }

        return true;
    }

   public:
    static NetStatusManager* getIntance()
    {
        static NetStatusManager obj;
        return &obj;
    }

    void setCheck4(bool en)
    {
        en4 = en;
    }

    void setCheck6(bool en)
    {
        en6 = en;
    }

    bool wan4_is_good()
    {
        return good4 ? true : false;
    }
    bool wan6_is_good()
    {
        return good6 ? true : false;
    }
};

#define NET_STATUS NetStatusManager::getIntance()

#endif  // __NETSTATUS_MANAGER_H__
