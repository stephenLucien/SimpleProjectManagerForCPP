#ifndef __PTHREAD_MUTEX_H__
#define __PTHREAD_MUTEX_H__


#include <pthread.h>

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
    PthreadMutex(const PthreadMutex &)            = delete;  // Disable copy constructor
    PthreadMutex &operator=(const PthreadMutex &) = delete;  // Disable copy

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
        inline Writelock(PthreadMutex &mutex) : mLock(mutex)
        {
            mLock.w_lock();
        }

        /**
         * @brief 构造时加锁
         */
        inline Writelock(PthreadMutex *mutex) : mLock(*mutex)
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
        PthreadMutex &mLock;
    };



    class Readlock
    {
       public:
        inline Readlock(PthreadMutex &mutex) : mLock(mutex)
        {
            mLock.r_lock();
        }

        /**
         * @brief 构造时加锁
         */
        inline Readlock(PthreadMutex *mutex) : mLock(*mutex)
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
        PthreadMutex &mLock;
    };
};


#endif  // __PTHREAD_MUTEX_H__
