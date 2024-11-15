#ifndef __PTHREAD_CLASS_H__
#define __PTHREAD_CLASS_H__

#include <pthread.h>
#include <unistd.h>
#include <string>

#include "os_tools.h"

class PthreadWrapper
{
   private:
    int is_running;
    //
    int req_exit;
    //
    pthread_t tid;
    //
    std::string name;
    //
    int m_task_cnt;

    //
    void exec_loop()
    {
        // wait until ready
        while (true)
        {
            if (readyToRun())
            {
                break;
            }
            if (exitPending())
            {
                return;
            }
            usleep(100);
        }

        //
        while (true)
        {
            if (!threadLoop())
            {
                return;
            }
            if (exitPending())
            {
                return;
            }
            usleep(0);
        }
    }

    void wait_loop(int timeout_ms = 30 * 1000)
    {
        const int duration_ms = 50;
        do
        {
            if (timeout_ms <= 0)
            {
                break;
            }
            if (!is_running)
            {
                break;
            }
            int wait_ms = timeout_ms > duration_ms ? duration_ms : timeout_ms;
            usleep(1000 * wait_ms);
            timeout_ms -= wait_ms;
        } while (true);
        cancel_task();
    }

    static void *thread_task(void *userdata)
    {
        pthread_detach(pthread_self());
        //
        auto pobj = (PthreadWrapper *)userdata;
        //
        if (!pobj)
        {
            pthread_exit(NULL);
        }
        if (!pobj->name.empty())
        {
            pthread_setname_np(pobj->tid, pobj->name.c_str());
        }

        pobj->exec_loop();

        pobj->is_running = 0;

        //
        --(pobj->m_task_cnt);

        pthread_exit(NULL);
    }

    void cancel_task()
    {
        if (is_running)
        {
            OS_PRINT("");
            pthread_cancel(tid);
            is_running = 0;
        }
    }
    /**
     * @brief 启动线程
     * @param name 线程名称；默认为NULL，由系统自动分配
     */
    bool begin_loop()
    {
        if (is_running)
        {
            // OS_PRINT("%s: runing", __PRETTY_FUNCTION__);
            return false;
        }

        is_running = 1;
        //
        int ret = pthread_create(&tid, NULL, thread_task, this);
        //
        if (ret)
        {
            OS_PRINT("%s: run failed", __PRETTY_FUNCTION__);
            is_running = 0;
            return false;
        }
        ++m_task_cnt;
        return true;
    }

   public:
    /**
     * @brief 启动线程
     * @param name 线程名称；默认为NULL，由系统自动分配
     */
    bool run(const char *name = NULL)
    {
        if (name)
        {
            this->name = name;
        }

        return begin_loop();
    }

    /**
     * @brief 线程是否运行中
     */
    bool isRunning()
    {
        return is_running;
    }

    /**
     * @brief 请求退出线程
     * @attention 调用完函数立即返回，并不代表线程也退出了
     */
    void requestExit()
    {
        req_exit = 1;
    }

    /**
     * @brief 请求并等待线程退出
     * @attention 线程退出，函数才返回
     */
    void requestExitAndWait()
    {
        req_exit = 1;
        wait_loop();
    }

   public:
    PthreadWrapper()
    {
        is_running = 0;
        //
        req_exit = 0;
        //
        name = std::string();
        //
        m_task_cnt = 0;
    }
    virtual ~PthreadWrapper()
    {
        req_exit = 1;
        wait_loop();
    }

    // Disable copy constructor
    PthreadWrapper(const PthreadWrapper &) = delete;
    // Disable copy
    PthreadWrapper &operator=(const PthreadWrapper &) = delete;


    static void sleep(int msec)
    {
        msec = msec >= 0 ? msec : 0;
        usleep(msec * 1000);
    }

    void cancel()
    {
        cancel_task();
    }

   protected:
    /**
     * @brief 是否有退出线程请求
     */
    virtual bool exitPending()
    {
        // OS_PRINT("%s", __PRETTY_FUNCTION__);
        return req_exit;
    }

    /**
     * @brief 线程开始运行时回调该接口
     */
    virtual bool readyToRun()
    {
        // OS_PRINT("%s", __PRETTY_FUNCTION__);
        return true;
    }

    /**
     * @brief 线程循环调用该接口
     * @return true 不退出线程，false 将退出线程
     */
    virtual bool threadLoop()
    {
        // OS_PRINT("%s", __PRETTY_FUNCTION__);
        usleep(1000 * 1000);
        return true;
    }
};

#endif  // __PTHREAD_CLASS_H__
