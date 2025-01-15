#ifndef __OS_TOOLS_SELECT_H__
#define __OS_TOOLS_SELECT_H__

//
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>
#include "utils/os_tools_log.h"
#include "utils/pthread_class.hpp"

class EpollHelper : PthreadWrapper
{
   private:
    int epoll_fd;
    //
    int check_interval_ms = 50;
    //
    std::vector<struct epoll_event> events;
    //
    PthreadMutex m_mtx;
    //
    class CallbackData
    {
       public:
        CallbackData(std::function<void(int fd, int event, void* userdata)> func = NULL, void* data = NULL) : func(std::move(func)), data(data)
        {
        }
        void run(int fd, uint32_t event)
        {
            if (func)
            {
                func(fd, event, data);
            }
        }
        //
        std::function<void(int, uint32_t, void*)> func;
        //
        void* data;
    };
    std::unordered_map<int, CallbackData> m_cbs;
    //
    bool _fd_exists(int fd)
    {
        auto itr = m_cbs.find(fd);
        if (itr != m_cbs.end())
        {
            return true;
        }
        return false;
    }

    /**
     * @brief
     *
     * @param fd
     * @param events: events that is interested in.
     * it is a bitmask made up by enum EPOLL_EVENTS
     *
     * @return int
     */
    int _fd_add(int fd, uint32_t events, std::function<void(int fd, int event, void* userdata)> func, void* userdata)
    {
        if (!_isValid())
        {
            return -1;
        }
        struct epoll_event st_event;
        //
        st_event.events  = events;
        st_event.data.fd = fd;
        //
        auto b_exist = _fd_exists(fd);
        //
        auto ret = epoll_ctl(epoll_fd, b_exist ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &st_event);
        if (ret)
        {
            OS_LOGE("errno=%d: %s", errno, strerror(errno));
            return ret;
        }
        OS_LOGI("%s fd=%d", b_exist ? "MOD" : "ADD", fd);
        m_cbs[fd] = CallbackData(std::move(func), userdata);

        return 0;
    }

    int _fd_del(int fd)
    {
        if (_fd_exists(fd))
        {
            auto ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            if (ret)
            {
                OS_LOGE("errno=%d: %s", errno, strerror(errno));
            }
            m_cbs.erase(fd);
            return 1;
        }
        return 0;
    }

    int _fd_cb_run(int fd, uint32_t event)
    {
        auto itr = m_cbs.find(fd);
        if (itr == m_cbs.end())
        {
            OS_LOGV("fd: %d", fd);
            return -1;
        }
        itr->second.run(fd, event);
        return 0;
    }

    void _fix_events_size()
    {
        //
        for (auto itr = m_cbs.begin(); itr != m_cbs.end();)
        {
            if (fd_closed(itr->first))
            {
                OS_LOGV("erase: fd=%d", itr->first);
                itr = m_cbs.erase(itr);
            } else
            {
                ++itr;
            }
        }
        events.resize(m_cbs.size() + 1);
    }

    bool _isValid()
    {
        return epoll_fd != -1;
    }

    int _epoll_close()
    {
        int ret = 0;
        if (_isValid())
        {
            ret = close(epoll_fd);
            //
            epoll_fd = -1;
        }
        return ret;
    }

    int _check(int timeout_ms = 0)
    {
        //
        int ret = -1;
        //
        if (!_isValid())
        {
            OS_LOGE("");
            return ret;
        }
        //
        _fix_events_size();

        ret = epoll_wait(epoll_fd, events.data(), (int)events.size(), timeout_ms);
        if (ret < 0)
        {
            OS_LOGE("errno=%d: %s", errno, strerror(errno));
            return ret;
        }
        // OS_LOGV("ret=%d", ret);

        for (int i = 0; i < ret; ++i)
        {
            // OS_LOGV("%d", i);
            _fd_cb_run(events[i].data.fd, events[i].events);
        }

        return ret;
    }

    //
    bool readyToRun()
    {
        return _isValid();
    }

    bool threadLoop()
    {
        msleep(check_interval_ms);
        if (exitPending())
        {
            return false;
        }
        if (!_isValid())
        {
            return false;
        }
        check();

        return true;
    }

   public:
    EpollHelper(int epoll_flag = EPOLL_CLOEXEC)
    {
        epoll_fd = epoll_create1(epoll_flag);
        //
        events.clear();
        //
        _fix_events_size();
    }
    ~EpollHelper()
    {
        _epoll_close();
    }

    bool fd_exists(int fd)
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _fd_exists(fd);
    }
    /**
     * @brief
     *
     * @param fd
     * @param events: events that is interested in.
     * it is a bitmask made up by enum EPOLL_EVENTS
     *
     * @return int
     */
    int fd_add(int fd, uint32_t events, std::function<void(int fd, int event, void* userdata)> func, void* userdata)
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _fd_add(fd, events, std::move(func), userdata);
    }

    int fd_del(int fd)
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _fd_del(fd);
    }

    bool isValid()
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _isValid();
    }

    int epoll_close()
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _epoll_close();
    }

    int check(int timeout_ms = 0)
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _check(timeout_ms);
    }

    static int fd_accessable(int fd, int* err)
    {
        if (err)
        {
            *err = 0;
        }
        struct stat buf;
        if (fstat(fd, &buf) == -1)
        {
            if (err)
            {
                *err = errno;
            }

            if (errno == EBADF)
            {
                // fd is bad.
            }
            // OS_LOGE("errno=%d: %s", errno, strerror(errno));
            return 0;
        }
        return 1;
    }

    static int fd_closed(int fd)
    {
        int err;
        if (!fd_accessable(fd, &err) && err == EBADF)
        {
            return 1;
        }
        return 0;
    }

    static int fd_set_block(int fd, int blocked = 1)
    {
        if (fd == -1)
        {
            return -1;
        }
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0)
        {
            OS_LOGD("Get flags error:%s\n", strerror(errno));
            return -1;
        }
        //
        auto mask = flags;
        //
        mask  = -1;
        mask ^= O_NONBLOCK;
        //
        auto nflags = flags & mask;
        if (blocked == 0)
        {
            nflags |= O_NONBLOCK;
        }

        OS_LOGV("flags: 0x%X --> 0x%X", flags, nflags);
        if (fcntl(fd, F_SETFL, nflags) < 0)
        {
            OS_LOGD("Set flags error:%s\n", strerror(errno));
            return -1;
        }

        return 0;
    }

    static int fd_timeout_event(int* pret, int fd, int timeout_ms, std::function<void(int fd, int event, void* pret)> func)
    {
        EpollHelper ep;
        //
        ep.fd_add(fd, EPOLLIN | EPOLLOUT, std::move(func), pret);
        //
        return ep.check(timeout_ms);
    }

    static int fd_timeout_read(int* pret, int fd, int timeout_ms, std::function<void(int fd, int event, void* pret)> func)
    {
        EpollHelper ep;
        //
        ep.fd_add(fd, EPOLLIN, std::move(func), pret);
        //
        return ep.check(timeout_ms);
    }

    static int fd_timeout_write(int* pret, int fd, int timeout_ms, std::function<void(int fd, int event, void* pret)> func)
    {
        EpollHelper ep;
        //
        ep.fd_add(fd, EPOLLOUT, std::move(func), pret);
        //
        return ep.check(timeout_ms);
    }
};


#endif  // __OS_TOOLS_SELECT_H__
