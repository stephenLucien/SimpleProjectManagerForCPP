#ifndef __CPPHELPER_PTHREAD_H__
#define __CPPHELPER_PTHREAD_H__


#include <inttypes.h>
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>


#include "utils/os_tools_log.h"



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

    /**
     * @brief
     *
     * @return int
     * - 0 : success
     */
    int w_lock()
    {
        // \ref https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_rwlock_wrlock.html
        return pthread_rwlock_wrlock(&rwlock);
    }
    /**
     * @brief
     *
     * @return int
     * - 0 : success
     */
    int w_trylock()
    {
        // \ref https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_rwlock_trywrlock.html
        return pthread_rwlock_trywrlock(&rwlock);
    }
    /**
     * @brief
     *
     * @param timeout_ms
     * @return int
     * - 0 : success
     */
    int w_timedlock(int timeout_ms)
    {
        if (timeout_ms == 0)
        {
            return w_trylock();
        }
        if (timeout_ms < 0)
        {
            return w_lock();
        }
        struct timespec abstime;
        //
        abstime.tv_sec = timeout_ms / 1000;
        //
        timeout_ms %= 1000;
        //
        abstime.tv_nsec = timeout_ms * 1000 * 1000;
        // \ref https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_rwlock_timedwrlock.html
        return pthread_rwlock_timedwrlock(&rwlock, &abstime);
    }
    /**
     * @brief
     *
     * @return int
     * - 0 : success
     */
    int r_lock()
    {
        // \ref https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_rwlock_rdlock.html
        return pthread_rwlock_rdlock(&rwlock);
    }
    /**
     * @brief
     *
     * @return int
     * - 0 : success
     */
    int r_trylock()
    {
        // \ref https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_rwlock_tryrdlock.html
        return pthread_rwlock_tryrdlock(&rwlock);
    }
    /**
     * @brief
     *
     * @param timeout_ms
     * @return int
     * - 0 : success
     */
    int r_timedlock(int timeout_ms)
    {
        if (timeout_ms == 0)
        {
            return r_trylock();
        }
        if (timeout_ms < 0)
        {
            return r_lock();
        }
        struct timespec abstime;
        //
        abstime.tv_sec = timeout_ms / 1000;
        //
        timeout_ms %= 1000;
        //
        abstime.tv_nsec = timeout_ms * 1000 * 1000;
        // \ref https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_rwlock_timedrdlock.html
        return pthread_rwlock_timedrdlock(&rwlock, &abstime);
    }
    int unlock()
    {
        return pthread_rwlock_unlock(&rwlock);
    }


    class Writelock
    {
       public:
        inline Writelock(PthreadMutex& mutex, int timeout_ms = -1) : mLock(mutex)
        {
            ret = mLock.w_timedlock(timeout_ms);
        }

        inline Writelock(PthreadMutex* mutex, int timeout_ms = -1) : mLock(*mutex)
        {
            ret = mLock.w_timedlock(timeout_ms);
        }

        inline ~Writelock()
        {
            // only release mutex on got mutex
            if (got())
            {
                mLock.unlock();
            }
        }

        /**
         * @brief
         *
         * @return true : got mutex
         * @return false : got mutex fail
         */
        bool got()
        {
            return ret == 0;
        }

       private:
        //
        PthreadMutex& mLock;
        //
        int ret = -1;
    };

    class Readlock
    {
       public:
        inline Readlock(PthreadMutex& mutex, int timeout_ms = -1) : mLock(mutex)
        {
            ret = mLock.r_timedlock(timeout_ms);
        }

        inline Readlock(PthreadMutex* mutex, int timeout_ms = -1) : mLock(*mutex)
        {
            ret = mLock.r_timedlock(timeout_ms);
        }

        inline ~Readlock()
        {
            // only release mutex on got mutex
            if (got())
            {
                mLock.unlock();
            }
        }

        /**
         * @brief
         *
         * @return true : got mutex
         * @return false : got mutex fail
         */
        bool got()
        {
            return ret == 0;
        }

       private:
        //
        PthreadMutex& mLock;
        //
        int ret = -1;
    };
};

class PthreadWrapper
{
   private:
    //
    PthreadMutex m_mtx;
    /**
     * @brief pthread identifier
     * - detached thread: valid until thread terminated.
     * - attached thread: valid until pthread_join().
     *
     */
    pthread_t tid;
    /**
     * @brief
     */
    int tid_valid;
    /**
     * @brief routine name
     *
     */
    std::string name;

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
     * @brief when the destructor called,
     * wait for normal exit at most `timeout_normal_exit` miliseconds before
     * trigger cancel()
     *
     */
    int32_t timeout_normal_exit = 60 * 1000;

    /**
     * @brief check whether routine should be terminated
     */
    bool _exitPending()
    {
        // OS_LOGV("%s", __PRETTY_FUNCTION__);
        return req_exit;
    }

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
            if (_exitPending())
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
        pthread_cleanup_push(pthread_routine_cleanup_impl, pobj);

        if (!pobj->name.empty())
        {
            pthread_setname_np(pobj->tid, pobj->name.c_str());
        }
        enable_cancel();

        OS_LOGI("thread(name='%s', pid=%lu) BEGIN\n", pobj->name.c_str(), pobj->tid);

        pobj->exec_loop();

        // \ref https://linux.die.net/man/3/pthread_cleanup_pop
        pthread_cleanup_pop(1);

        pthread_exit(NULL);
    }

    /**
     * @brief terminate routine
     * - if routine is running: trigger pthread_cancel()
     *
     * @param detach :
     * - true : detach routine
     * - false : join routine
     * @return int
     * - 0 : no error
     * - other :
     * @note: `tid_valid` will be reset
     */
    int cancel_task(bool detach = false)
    {
        int ret = 0;
        //
        req_exit = 1;
        //
        if (tid_valid == 0)
        {
            if (is_running)
            {
                OS_LOGW("thread(name='%s', pid=%lu) running, but detach?\n", name.c_str(), tid);
            }
            return ret;
        }

        if (is_running)
        {
            /**
             * @brief
             * @note cancel before detach, since tid may be invalid if detached thread is terminated.
             * @note just send a signal, routine will be terminated at next cancellation point.
             *
             */
            ret = pthread_cancel(tid);
            OS_LOGW("%s: pthread_cancel(pid=%lu) ret=%d, name=%s\n", __PRETTY_FUNCTION__, tid, ret, this->name.c_str());
        }

        if (detach)
        {
            ret = pthread_detach(tid);
            OS_LOGI("%s: pthread_detach(pid=%lu) ret=%d, name=%s\n", __PRETTY_FUNCTION__, tid, ret, this->name.c_str());
        } else
        {
            if (is_running)
            {
                OS_LOGV("thread(name='%s', pid=%lu) joining, may take some time!\n", name.c_str(), tid);
            }
            ret = pthread_join(tid, NULL);
            OS_LOGI("%s: pthread_join(pid=%lu) ret=%d, name=%s\n", __PRETTY_FUNCTION__, tid, ret, this->name.c_str());
        }

        tid_valid = 0;

        return ret;
    }

    /**
     * @brief create routine and run
     */
    bool begin_loop()
    {
        int ret = -1;
        //
        cancel_task(false);
        //
        if (is_running)
        {
            return false;
        }
        if (!readyToRun())
        {
            return false;
        }

        //
        req_exit = 0;
        //
        is_running = 1;
        //
        ret = pthread_create(&tid, NULL, pthread_routine_running_impl, this);
        //
        if (ret)
        {
            OS_LOGV("%s: pthread_create(pid=%lu) fail, name=%s\n", __PRETTY_FUNCTION__, tid, this->name.c_str());
            is_running = 0;
            return false;
        } else
        {
            tid_valid = 1;
            OS_LOGV("%s: pthread_create(pid=%lu) success, name=%s\n", __PRETTY_FUNCTION__, tid, this->name.c_str());
        }

        return true;
    }

   public:
    PthreadWrapper(const std::string& taskName = std::string())
    {
        tid_valid = 0;
        //
        is_running = 0;
        //
        req_exit = 0;
        //
        setTaskName(taskName);
    }
    virtual ~PthreadWrapper()
    {
        if (is_detached())
        {
            OS_LOGV("thread(name='%s', pid=%lu) requestExitAndWait\n", name.c_str(), tid);
            // since the rountine refer to this obj, we block the destructor incase of segmentation fault
            requestExitAndWait(INT32_MAX);
            if (is_running)
            {
                OS_LOGV("thread(name='%s', pid=%lu) oops!!! crash...\n", name.c_str(), tid);
                fflush(stderr);
                fflush(stdout);
            }
        } else
        {
            // try normal exit first
            requestExitAndWait(timeout_normal_exit);
            // try cancellation point
            cancel_task(false);
        }
    }

    /**
     * @brief sleep for milisecond
     *
     * @param msec
     */
    static void msleep(int msec)
    {
        msec = msec >= 0 ? msec : 0;
        usleep(msec * 1000);
    }

    void msleep_sliced(int msec, int interval_ms = 10)
    {
        if (msec < 0)
        {
            msec = 0;
        }
        if (interval_ms <= 0)
        {
            interval_ms = 10;
        }
        auto cnt = msec / interval_ms;
        auto rem = msec % interval_ms;
        for (; cnt; --cnt)
        {
            if (_exitPending())
            {
                return;
            }
            if (exitPending())
            {
                return;
            }
            msleep(interval_ms);
        }
        msleep(rem);
    }

    /**
     * @brief sleep for milisecond
     *
     * @param msec
     */
    static void sleep(int msec)
    {
        msleep(msec);
    }

    // Disable copy constructor
    PthreadWrapper(const PthreadWrapper&) = delete;
    // Disable copy
    PthreadWrapper& operator=(const PthreadWrapper&) = delete;

    void setTaskName(const std::string& taskName)
    {
        this->name = taskName;
    }

    void setNormalExitTimeout(int32_t ms)
    {
        timeout_normal_exit = ms;
    }

    bool run(const char* task_name = NULL)
    {
        PthreadMutex::Writelock lock(m_mtx);
        if (task_name)
        {
            setTaskName(task_name);
        }
        return begin_loop();
    }

    bool tryRun(const char* task_name = NULL)
    {
        if (isRunning())
        {
            return false;
        }
        return run(task_name);
    }

    bool isRunning()
    {
        return is_running;
    }

    void _requestExit()
    {
        req_exit = 1;
    }

    virtual void requestExit()
    {
    }

    /**
     * @brief
     *
     * @param timeout_ms : wait for timeout
     * @return true : task end
     * @return false : task running
     */
    bool wait_loop(int32_t timeout_ms = 30 * 1000)
    {
        const int32_t duration_ms = 50;
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
            auto wait_ms = timeout_ms > duration_ms ? duration_ms : timeout_ms;
            usleep(1000 * wait_ms);
            timeout_ms -= wait_ms;
        } while (true);

        return !(is_running);
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

    bool is_detached()
    {
        return tid_valid == 0 && is_running;
    }

    /**
     * @brief terminate routine at next cancellation point.
     * routine may be terminated faster then normal exit.
     *
     * @note it is a good practice not to use cancellation point.
     * @note CAUTION!!! use it carefully, especially when the routine
     * uses heap memory. Setup pthread_cleanup_push() && pthread_cleanup_pop() in routine
     * whenever you consider calling this function.
     * \ref https://man7.org/linux/man-pages/man3/pthread_cleanup_push.3.html
     *
     * @param detach :
     * - true : trigger pthread_cancel() and immediately return
     * - false: trigger pthread_cancel() and wait until routine terminated
     */
    void cancel(bool detach = false)
    {
        PthreadMutex::Writelock lock(m_mtx);
        cancel_task(detach);
    }


    /**
     * @brief request exit and wait for timeout_ms until routine end.
     *
     * @param timeout_ms
     * @return true : terminated
     * @return false : still running
     */
    bool requestExitAndWait(int32_t timeout_ms = INT32_MAX)
    {
        //
        _requestExit();
        //
        requestExit();
        //
        if (is_running)
        {
            OS_LOGV("timeout_ms=%" PRId32 "", timeout_ms);
            //
            wait_loop(timeout_ms);
        }
        return !(is_running);
    }

   protected:
    /**
     * @brief check whether routine should be terminated
     */
    virtual bool exitPending()
    {
        // OS_LOGV("%s", __PRETTY_FUNCTION__);
        return false;
    }

    /**
     * @brief call before routine created
     */
    virtual bool readyToRun()
    {
        // OS_LOGV("%s", __PRETTY_FUNCTION__);
        return true;
    }

    /**
     * @brief
     *
     * @return true : continue routine
     * @return false : break routine
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


#define PTHREAD_FOPEN(fd, path, mode)                                                           \
    int                                    fd##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<FILE, void (*)(FILE*)> fd##_cpp_ptr(fopen(path, mode),                      \
                                                        [](FILE* ptr)                           \
                                                        {                                       \
                                                            if (ptr)                            \
                                                            {                                   \
                                                                fclose(ptr);                    \
                                                            }                                   \
                                                        });                                     \
    PthreadWrapper::setcancelstate(fd##_cancelstate);                                           \
    FILE* fd = fd##_cpp_ptr.get()

#define PTHREAD_POPEN(fd, cmd, mode)                                                            \
    int                                    fd##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<FILE, void (*)(FILE*)> fd##_cpp_ptr(popen(cmd, mode),                       \
                                                        [](FILE* ptr)                           \
                                                        {                                       \
                                                            if (ptr)                            \
                                                            {                                   \
                                                                fclose(ptr);                    \
                                                            }                                   \
                                                        });                                     \
    PthreadWrapper::setcancelstate(fd##_cancelstate);                                           \
    FILE* fd = fd##_cpp_ptr.get()

#define PTHREAD_OPENDIR(dir, path)                                                             \
    int                                  dir##_cancelstate = PthreadWrapper::disable_cancel(); \
    std::unique_ptr<DIR, void (*)(DIR*)> dir##_cpp_ptr(opendir(path),                          \
                                                       [](DIR* ptr)                            \
                                                       {                                       \
                                                           if (ptr)                            \
                                                           {                                   \
                                                               closedir(ptr);                  \
                                                           }                                   \
                                                       });                                     \
    PthreadWrapper::setcancelstate(dir##_cancelstate);                                         \
    DIR* dir = dir##_cpp_ptr.get()



#endif  // __CPPHELPER_PTHREAD_H__
