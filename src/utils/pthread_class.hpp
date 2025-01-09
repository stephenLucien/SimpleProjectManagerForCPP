#ifndef __PTHREAD_CLASS_H__
#define __PTHREAD_CLASS_H__

#include <pthread.h>
#include <unistd.h>
#include <string>

#include "os_tools_log.h"
#include "pthread_mutex.hpp"

class PthreadWrapper
{
   private:
    //
    PthreadMutex m_mtx;
    /**
     * @brief
     * - 0 : routine does not start.
     * - other : rountine had been created, should call pthread_join() or pthread_detach() to reclaim resources
     */
    int is_inited;
    /**
     * @brief routine running
     *
     */
    int is_running;
    /**
     * @brief request routine to terminate.
     *
     */
    int req_exit;
    /**
     * @brief pthread identifier
     * - detached thread: valid until thread terminated.
     * - attached thread: valid until pthread_join().
     *
     */
    pthread_t tid;
    /**
     * @brief routine name
     *
     */
    std::string name;

    //
    void exec_loop()
    {
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

    void wait_loop(int timeout_ms)
    {
        const int duration_ms = 50;
        do
        {
            if (timeout_ms <= 0)
            {
                break;
            }
            if (!is_inited)
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
    }

    /**
     * @brief pthread routine
     * @note must be terminated before destructor `~PthreadWrapper`,
     * since object pointer is being referred.
     *
     * @param userdata
     * @return void*
     */
    static void *thread_task(void *userdata)
    {
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

        pthread_exit(NULL);
    }

    /**
     * @brief terminate routine
     * - routine start & routine running: cancel and detach
     * - routine start & routine stop: join
     *
     * @return int
     * - 0 : no error
     * - other :
     * @note:
     * both `is_inited` and `is_running` will be reset
     */
    int cancel_task()
    {
        int ret = 0;
        //
        if (is_inited == 0)
        {
            is_running = 0;
            return ret;
        }

        if (is_running)
        {
            // cancel before detach, since tid may be invalid if detached thread is terminated.
            ret = pthread_cancel(tid);
            OS_LOGW("%s: pthread_cancel(pid=%lu) ret=%d, name=%s\n", __PRETTY_FUNCTION__, tid, ret, this->name.c_str());
            ret = pthread_detach(tid);
            OS_LOGW("%s: pthread_detach(pid=%lu) ret=%d, name=%s\n", __PRETTY_FUNCTION__, tid, ret, this->name.c_str());
            is_running = 0;
        } else
        {
            ret = pthread_join(tid, NULL);
            OS_LOGW("%s: pthread_join(pid=%lu) ret=%d, name=%s\n", __PRETTY_FUNCTION__, tid, ret, this->name.c_str());
        }

        is_inited = 0;
        return ret;
    }

    /**
     * @brief 启动线程
     */
    bool begin_loop()
    {
        if (is_inited && is_running)
        {
            return false;
        }

        cancel_task();

        if (!readyToRun())
        {
            return false;
        }

        //
        req_exit = 0;
        //
        is_running = 1;
        //
        int ret = pthread_create(&tid, NULL, thread_task, this);
        //
        if (ret)
        {
            OS_LOGV("%s: pthread_create(pid=%lu) fail, name=%s\n", __PRETTY_FUNCTION__, tid, this->name.c_str());
            is_running = 0;
            return false;
        } else
        {
            is_inited = 1;
            OS_LOGV("%s: pthread_create(pid=%lu) success, name=%s\n", __PRETTY_FUNCTION__, tid, this->name.c_str());
        }

        return true;
    }

   public:
    /**
     * @brief 启动线程
     * @param name 线程名称；默认为NULL，由系统自动分配
     */
    bool run(const char *name = NULL)
    {
        PthreadMutex::Writelock lock(m_mtx);
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
    void requestExitAndWait(int timeout_ms = 3000)
    {
        req_exit = 1;
        wait_loop(timeout_ms);
        cancel();
    }

   public:
    PthreadWrapper()
    {
        is_inited = 0;
        //
        is_running = 0;
        //
        req_exit = 0;
        //
        name = std::string();
    }
    virtual ~PthreadWrapper()
    {
        requestExitAndWait();
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
        PthreadMutex::Writelock lock(m_mtx);
        cancel_task();
    }

   protected:
    /**
     * @brief 是否有退出线程请求
     */
    virtual bool exitPending()
    {
        // OS_LOGV("%s", __PRETTY_FUNCTION__);
        return req_exit;
    }

    /**
     * @brief 线程开始运行时回调该接口
     */
    virtual bool readyToRun()
    {
        // OS_LOGV("%s", __PRETTY_FUNCTION__);
        return true;
    }

    /**
     * @brief 线程循环调用该接口
     * @return true 不退出线程，false 将退出线程
     */
    virtual bool threadLoop()
    {
        // OS_LOGV("%s", __PRETTY_FUNCTION__);
        usleep(1000 * 1000);
        return true;
    }
};
#endif  // __PTHREAD_CLASS_H__
