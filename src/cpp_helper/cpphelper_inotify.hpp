#ifndef __CPPHELPER_INOTIFY_H__
#define __CPPHELPER_INOTIFY_H__


#include <sys/inotify.h>
#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


#include "cpp_helper/cpphelper_epoll.hpp"
#include "cpp_helper/cpphelper_pthread.hpp"
#include "utils/os_tools_log.h"


class InotifyHelper : public PthreadWrapper
{
   private:
    //
    PthreadMutex m_mtx;
    //
    int m_fd = -1;
    //
    std::unordered_map<int, std::string> m_watch_fds;
    //
    std::string _getWatchName(int wd)
    {
        auto itr = m_watch_fds.find(wd);
        if (itr != m_watch_fds.end())
        {
            return itr->second;
        }
        return std::string();
    }
    //
    bool _isValid()
    {
        return !(m_fd < 0);
    }
    //
    int _rmWatch(int wd)
    {
        int ret = -1;
        //
        if (!(_isValid()))
        {
            return ret;
        }
        // \ref https://man7.org/linux/man-pages/man2/inotify_rm_watch.2.html
        ret = inotify_rm_watch(m_fd, wd);
        //
        if (ret == 0)
        {
            m_watch_fds.erase(wd);
            OS_LOGI("(m_fd=%d) remove watch on wd=%d", m_fd, wd);
        } else
        {
            OS_LOGE("(m_fd=%d) remove watch on wd=%d fail!", m_fd, wd);
        }

        return ret;
    }
    //
    int _rmWatch(const std::string &path)
    {
        bool found = false;
        //
        int wd = -1;
        //
        for (auto &tmpwd : m_watch_fds)
        {
            if (tmpwd.second == path)
            {
                wd = tmpwd.first;
                //
                found = true;
                break;
            }
        }

        if (!found)
        {
            return 0;
        }

        return _rmWatch(wd);
    }
    //
    int _rmWatchAll()
    {
        int ret = -1;
        //
        if (_isValid())
        {
            for (auto &wd : m_watch_fds)
            {
                // \ref https://man7.org/linux/man-pages/man2/inotify_rm_watch.2.html
                ret = inotify_rm_watch(m_fd, wd.first);
                //
                if (ret == 0)
                {
                    OS_LOGI("(m_fd=%d) remove watch on wd=%d, name=%s", m_fd, wd.first, wd.second.c_str());
                } else
                {
                    OS_LOGE("(m_fd=%d) remove watch on wd=%d, name=%s, fail!", m_fd, wd.first, wd.second.c_str());
                }
            }
        }
        m_watch_fds.clear();

        return ret;
    }
    //
    int _deinit()
    {
        int ret = -1;
        //
        _rmWatchAll();
        //
        if (!(_isValid()))
        {
            return 0;
        }

        ret = close(m_fd);
        //
        if (ret != 0)
        {
            OS_LOGE("close(%d), ret=%d", m_fd, ret);
        } else
        {
            OS_LOGI("close(%d), ret=%d", m_fd, ret);
        }
        //
        m_fd = -1;

        return ret;
    }
    //
    int _init()
    {
        int ret = -1;
        //
        _deinit();
        //
        m_fd = inotify_init();
        if (m_fd < 0)
        {
            OS_LOGE("inotify_init fail, ret=%d", ret);
            return ret;
        } else
        {
            OS_LOGI("inotify_init success, m_fd=%d", m_fd);
        }
        return 0;
    }
    //
    int _addWatch(const std::string &path, uint32_t mask = IN_CREATE | IN_MODIFY | IN_DELETE)
    {
        int ret = -1;
        //
        if (!(_isValid()))
        {
            return ret;
        }
        //
        int wd = inotify_add_watch(m_fd, path.c_str(), mask);
        //
        if (wd < 0)
        {
            OS_LOGE("(m_fd=%d) add watch to %s fail!\n", m_fd, path.c_str());
        } else
        {
            m_watch_fds[wd] = path;
            OS_LOGI("(m_fd=%d) add watch to %s\n", m_fd, path.c_str());
        }

        return ret;
    }

    size_t _estBufferSz()
    {
        //
        const size_t minsz = sizeof(struct inotify_event);
        //
        size_t bufsz = 0;
        //
        bufsz += (minsz + 256);
        //
        for (auto &e : m_watch_fds)
        {
            bufsz += (minsz + e.second.length() + 1);
        }
        return bufsz;
    }

    int _process(int timeout_ms = -1)
    {
        int ret = -1;
        //
        if (!(_isValid()))
        {
            return ret;
        }
        //
        std::vector<uint8_t> buffer_container(_estBufferSz());
        //
        auto buffer = buffer_container.data();
        //
        auto buffer_sz = buffer_container.size();

        //
        ssize_t length = -1;
        if (timeout_ms < 0)
        {
            length = read(m_fd, buffer, buffer_sz);
        } else
        {
            EpollHelper::fd_timeout_read(&length,
                                         m_fd,
                                         1000,
                                         [&](int fd, int event, void *pret)
                                         {
                                             auto plength = (ssize_t *)pret;
                                             //
                                             *plength = read(fd, buffer, buffer_sz);
                                         });
        }

        if (length < 0)
        {
            // OS_LOGE("length=%zd", length);
            return ret;
        }

        //
        ret = 0;
        //
        size_t offset = 0;
        //
        while (offset < length)
        {
            //
            struct inotify_event *event = (struct inotify_event *)&buffer[offset];
            //
            auto name = _getWatchName(event->wd);
            //
            _processEvents(event, name);
            //
            offset += sizeof(struct inotify_event) + event->len;
            //
            ++ret;
        }
        return ret;
    }

    int process(int timeout_ms = -1)
    {
        PthreadMutex::Writelock l(m_mtx);
        return _process(timeout_ms);
    }

    bool readyToRun() override
    {
        return _isValid();
    }

    bool threadLoop() override
    {
        if (exitPending())
        {
            return false;
        }
        //
        auto ret = process(1000);
        if (ret <= 0)
        {
            msleep_sliced(500);
        }

        //
        return true;
    }

   protected:
    // TODO:
    virtual int _processEvents(const struct inotify_event *event, const std::string &path)
    {
        int ret = -1;
        //
        if (!event)
        {
            return ret;
        }
        //
        OS_LOGV("wd=%d, mask=%08X, cookie=%08X, len=%08X, name=%s, path=%s",
                event->wd,
                event->mask,
                event->cookie,
                event->len,
                event->name,
                path.c_str());

        if (event->mask & IN_CREATE)
        {
            if (event->mask & IN_ISDIR)
            {
                printf("The directory %s was Created.\n", event->name);
            } else
            {
                printf("The file %s was Created with WD %d\n", event->name, event->wd);
            }
        }

        if (event->mask & IN_MODIFY)
        {
            if (event->mask & IN_ISDIR)
            {
                printf("The directory %s was modified.\n", event->name);
            } else
            {
                printf("The file %s was modified with WD %d\n", event->name, event->wd);
            }
        }

        if (event->mask & IN_DELETE)
        {
            if (event->mask & IN_ISDIR)
            {
                printf("The directory %s was deleted.\n", event->name);
            } else
            {
                printf("The file %s was deleted with WD %d\n", event->name, event->wd);
            }
        }

        return ret;
    }

   public:
    InotifyHelper()
    {
        init();
    }
    virtual ~InotifyHelper()
    {
        deinit();
    }
    //
    bool isValid()
    {
        PthreadMutex::Readlock l(m_mtx);
        return _isValid();
    }
    //
    int rmWatch(int wd)
    {
        PthreadMutex::Writelock l(m_mtx);
        return _rmWatch(wd);
    }
    //
    int rmWatch(const std::string &path)
    {
        PthreadMutex::Writelock l(m_mtx);
        return _rmWatch(path);
    }
    //
    int rmWatchAll()
    {
        PthreadMutex::Writelock l(m_mtx);
        return _rmWatchAll();
    }
    //
    int deinit()
    {
        PthreadMutex::Writelock l(m_mtx);
        return _deinit();
    }
    //
    int init()
    {
        PthreadMutex::Writelock l(m_mtx);
        return _init();
    }
    //
    int addWatch(const std::string &path, uint32_t mask = IN_CREATE | IN_MODIFY | IN_DELETE)
    {
        PthreadMutex::Writelock l(m_mtx);
        return _addWatch(path, mask);
    }
};

#endif  // __CPPHELPER_INOTIFY_H__
