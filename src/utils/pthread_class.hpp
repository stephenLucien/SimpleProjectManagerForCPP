#ifndef __PTHREAD_CLASS_H__
#define __PTHREAD_CLASS_H__

#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include <string>


#include "os_tools_log.h"



class PthreadMutex
{
   private:
    pthread_rwlock_t rwlock;

   public:
    PthreadMutex()
    {
        pthread_rwlock_init(&rwlock, NULL);
    }
    ~PthreadMutex()
    {
        pthread_rwlock_destroy(&rwlock);
    }
    PthreadMutex(const PthreadMutex&)            = delete;  // Disable copy constructor
    PthreadMutex& operator=(const PthreadMutex&) = delete;  // Disable copy

    void w_lock()
    {
        pthread_rwlock_wrlock(&rwlock);
    }
    void r_lock()
    {
        pthread_rwlock_rdlock(&rwlock);
    }
    void unlock()
    {
        pthread_rwlock_unlock(&rwlock);
    }


    class Writelock
    {
       public:
        inline Writelock(PthreadMutex& mutex) : mLock(mutex)
        {
            mLock.w_lock();
        }

        /**
         * @brief 构造时加锁
         */
        inline Writelock(PthreadMutex* mutex) : mLock(*mutex)
        {
            mLock.w_lock();
        }

        /**
         * @brief 析构时解锁
         */
        inline ~Writelock()
        {
            mLock.unlock();
        }

       private:
        PthreadMutex& mLock;
    };



    class Readlock
    {
       public:
        inline Readlock(PthreadMutex& mutex) : mLock(mutex)
        {
            mLock.r_lock();
        }

        /**
         * @brief 构造时加锁
         */
        inline Readlock(PthreadMutex* mutex) : mLock(*mutex)
        {
            mLock.r_lock();
        }

        /**
         * @brief 析构时解锁
         */
        inline ~Readlock()
        {
            mLock.unlock();
        }

       private:
        PthreadMutex& mLock;
    };
};

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

    static void pthread_routine_cleanup_impl(void* userdata)
    {
        auto pobj = (PthreadWrapper*)userdata;
        //
        if (!pobj)
        {
            return;
        }
        OS_LOGI("thread(name='%s', pid=%lu) END\n", pobj->name.c_str(), pobj->tid);
        pobj->is_running = 0;
    }

    /**
     * @brief pthread routine
     * @note must be terminated before destructor `~PthreadWrapper`,
     * since object pointer is being referred.
     *
     * @param userdata
     * @return void*
     */
    static void* pthread_routine_running_impl(void* userdata)
    {
        //
        auto pobj = (PthreadWrapper*)userdata;
        //
        if (!pobj)
        {
            pthread_exit(NULL);
        }

        if (!pobj->name.empty())
        {
            pthread_setname_np(pobj->tid, pobj->name.c_str());
        }
        enable_cancel();

        OS_LOGI("thread(name='%s', pid=%lu) BEGIN\n", pobj->name.c_str(), pobj->tid);
        pthread_cleanup_push(pthread_routine_cleanup_impl, pobj);

        pobj->exec_loop();

        // \ref https://linux.die.net/man/3/pthread_cleanup_pop
        pthread_cleanup_pop(1);

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
     * @brief create routine and run
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
        int ret = pthread_create(&tid, NULL, pthread_routine_running_impl, this);
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

    /**
     * @brief sleep for milisecond
     *
     * @param msec
     */
    static void sleep(int msec)
    {
        msec = msec >= 0 ? msec : 0;
        usleep(msec * 1000);
    }

    // Disable copy constructor
    PthreadWrapper(const PthreadWrapper&) = delete;
    // Disable copy
    PthreadWrapper& operator=(const PthreadWrapper&) = delete;

    bool run(const char* task_name = NULL)
    {
        PthreadMutex::Writelock lock(m_mtx);
        if (task_name)
        {
            this->name = task_name;
        }
        return begin_loop();
    }

    bool isRunning()
    {
        return is_running;
    }

    void requestExit()
    {
        req_exit = 1;
    }

    /**
     * @brief
     *
     * @param timeout_ms : wait for timeout
     * @return true : task end
     * @return false : task running
     */
    bool wait_loop(int timeout_ms = 30 * 1000)
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
        return !(is_inited && is_running);
    }

    /**
     * @brief
     *
     */
    static int enable_cancel()
    {
        int oldstate;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
        return oldstate;
    }
    /**
     * @brief useful when doing heap operations. such as malloc()
     *
     */
    static int disable_cancel()
    {
        int oldstate;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
        return oldstate;
    }
    /**
     * @brief
     *
     * @param state :
     * - PTHREAD_CANCEL_ENABLE
     * - PTHREAD_CANCEL_DISABLE
     * @return int
     */
    static int setcancelstate(int state)
    {
        int oldstate;
        pthread_setcancelstate(state, &oldstate);
        return oldstate;
    }

    void cancel()
    {
        PthreadMutex::Writelock lock(m_mtx);
        cancel_task();
    }

    /**
     * @brief 请求并等待线程退出
     * @attention 线程退出，函数才返回
     */
    void requestExitAndWait(int timeout_ms = 3000)
    {
        requestExit();
        wait_loop(timeout_ms);
        cancel();
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

class TmpDisablePthreadCancel
{
   private:
    int oldstate;

   public:
    TmpDisablePthreadCancel()
    {
        oldstate = PthreadWrapper::disable_cancel();
    }
    ~TmpDisablePthreadCancel()
    {
        PthreadWrapper::setcancelstate(oldstate);
    }
};


#define PTHREAD_MALLOC(type, name, cnt)                                                           \
    size_t                                 name##_sz          = sizeof(type) * (cnt);             \
    int                                    name##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<type, void (*)(type*)> name##_cpp_ptr(static_cast<type*>(malloc(name##_sz)),  \
                                                          [](type* ptr)                           \
                                                          {                                       \
                                                              if (ptr)                            \
                                                              {                                   \
                                                                  free(ptr);                      \
                                                              }                                   \
                                                          });                                     \
    PthreadWrapper::setcancelstate(name##_cancelstate);                                           \
    type* name = name##_cpp_ptr.get()

#define PTHREAD_CALLOC(type, name, cnt)                                                                  \
    size_t                                 name##_sz          = sizeof(type) * (cnt);                    \
    int                                    name##_cancelstate = PthreadWrapper::disable_cancel();        \
    std::unique_ptr<type, void (*)(type*)> name##_cpp_ptr(static_cast<type*>(calloc(cnt, sizeof(type))), \
                                                          [](type* ptr)                                  \
                                                          {                                              \
                                                              if (ptr)                                   \
                                                              {                                          \
                                                                  free(ptr);                             \
                                                              }                                          \
                                                          });                                            \
    PthreadWrapper::setcancelstate(name##_cancelstate);                                                  \
    type* name = name##_cpp_ptr.get()


#define PTHREAD_FOPEN(fd, path, mode)                                                             \
    int                                    name##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<FILE, void (*)(FILE*)> fd##_cpp_ptr(fopen(path, mode),                        \
                                                        [](FILE* ptr)                             \
                                                        {                                         \
                                                            if (ptr)                              \
                                                            {                                     \
                                                                fclose(ptr);                      \
                                                            }                                     \
                                                        });                                       \
    PthreadWrapper::setcancelstate(name##_cancelstate);                                           \
    FILE* fd = fd##_cpp_ptr.get()

#define PTHREAD_POPEN(fd, cmd, mode)                                                              \
    int                                    name##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<FILE, void (*)(FILE*)> fd##_cpp_ptr(popen(cmd, mode),                         \
                                                        [](FILE* ptr)                             \
                                                        {                                         \
                                                            if (ptr)                              \
                                                            {                                     \
                                                                fclose(ptr);                      \
                                                            }                                     \
                                                        });                                       \
    FILE*                                  fd = fd##_cpp_ptr.get()

#define PTHREAD_OPENDIR(dir, path)                                                              \
    int                                  name##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<DIR, void (*)(DIR*)> dir##_cpp_ptr(opendir(path),                           \
                                                       [](DIR* ptr)                             \
                                                       {                                        \
                                                           if (ptr)                             \
                                                           {                                    \
                                                               closedir(ptr);                   \
                                                           }                                    \
                                                       });                                      \
    PthreadWrapper::setcancelstate(name##_cancelstate);                                         \
    DIR* dir = dir##_cpp_ptr.get()



#endif  // __PTHREAD_CLASS_H__
