#include "buffer_manager.hpp"

//
#include <cstdint>
#include <list>
#include <vector>

//
#include "cpp_helper/cpphelper_event_marker.hpp"
#include "cpp_helper/cpphelper_pthread.hpp"
#include "utils/os_tools.h"

//
#if defined(OS_LOG_TAG)
    #undef OS_LOG_TAG
    #define OS_LOG_TAG "BufM"
#endif



void BufferManager::BufferItem::init(void *buf, int bufsz, bool b_copy, void *pool)
{
    this->isCopy = b_copy;
    if (b_copy)
    {
        if (buf && bufsz > 0)
        {
            this->pool = calloc(1, bufsz);
            //
            this->bufsz = bufsz;
            //
            this->buf = this->pool;
        } else
        {
            //
            this->buf = NULL;
            //
            this->bufsz = 0;
            //
            this->pool = NULL;
        }
    } else
    {
        //
        this->buf = buf;
        //
        this->bufsz = bufsz;
        //
        this->pool = pool;
    }
}
void BufferManager::BufferItem::deinit()
{
    if (!isCopy)
    {
        return;
    }
    if (this->pool)
    {
        free(this->pool);
        this->pool = NULL;
        this->buf  = NULL;
    }
}
BufferManager::BufferItem::BufferItem(void *buf, int bufsz, bool b_copy, void *pool)
{
    init(buf, bufsz, b_copy, pool);
}
BufferManager::BufferItem::~BufferItem()
{
    deinit();
}
void *BufferManager::BufferItem::getPool()
{
    return this->pool;
}
bool BufferManager::BufferItem::isValid()
{
    return buf && bufsz > 0;
}
void BufferManager::BufferItem::dump(const std::string &tagstr)
{
    OS_LOGV("<%s>[p:%p%s] ptr: %p, sz: %d", tagstr.c_str(), pool, isCopy ? ",copied" : "", buf, bufsz);
}



BufferManager::BufferManager(int slice_sz, int total_cnt, BufferItemHDL func, void *func_data, int cb_duration_ms)
{
    setTaskName("BufferManager");
    //
    _setup(slice_sz, total_cnt, func, func_data, cb_duration_ms);
}

BufferManager::~BufferManager()
{
}


int BufferManager::_setup(int slice_sz, int total_cnt, BufferItemHDL func, void *func_data, int cb_duration_ms)
{
    //
    this->slice_sz = slice_sz;
    this->total_sz = slice_sz * total_cnt;
    //
    this->cb = func;
    //
    this->cb_data = func_data;
    //
    this->cb_duration_ms = cb_duration_ms;
    //
    used_buffers.clear();
    //
    buffer.resize(total_sz);
    //
    availabe_buffers.clear();
    for (int pos = 0; pos + slice_sz <= total_sz; pos += slice_sz)
    {
        availabe_buffers.emplace_back(BufferItem(buffer.data() + pos, slice_sz, false, buffer.data()));
    }
    return 0;
}


int BufferManager::dropCache()
{
    std::list<BufferItem> tmpUsedBuffers;
    {
        //
        PthreadMutex::Writelock _l(used_buffers_lock);
        tmpUsedBuffers = used_buffers;
        used_buffers.clear();
    }

    {
        //
        PthreadMutex::Writelock _l(availabe_buffers_lock);
        for (auto &e : tmpUsedBuffers)
        {
            availabe_buffers.push_back(e);
        }
    }
    return 0;
}

BufferManager::BufferItem BufferManager::getBuffer(int timeout_ms)
{
    BufferItem buffer;
    //
    if (!isRunning())
    {
        return buffer;
    }
    //
    PthreadMutex::Writelock _l(availabe_buffers_lock, timeout_ms);
    if (!_l.got())
    {
        return buffer;
    }
    if (availabe_buffers.empty())
    {
        return buffer;
    }
    buffer = availabe_buffers.front();
    availabe_buffers.pop_front();

    // OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
    return buffer;
}

void BufferManager::pushData(BufferItem buffer)
{
    if (!buffer.isValid())
    {
        return;
    }
    PthreadMutex::Writelock _l(used_buffers_lock);
    if (buffer.getPool() == this->buffer.data())
    {
        used_buffers.push_back(buffer);
        // OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
    } else
    {
        OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
    }
}

BufferManager::BufferItem BufferManager::popData()
{
    BufferItem buffer = {0};
    //
    PthreadMutex::Writelock _l(used_buffers_lock);
    if (!_l.got())
    {
        return buffer;
    }
    if (used_buffers.empty())
    {
        return buffer;
    }
    buffer = used_buffers.front();
    used_buffers.pop_front();

    // OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
    return buffer;
}

void BufferManager::ReleaseBuffer(BufferItem buffer)
{
    if (!buffer.isValid())
    {
        return;
    }
    PthreadMutex::Writelock _l(availabe_buffers_lock);
    if (buffer.getPool() == this->buffer.data())
    {
        availabe_buffers.push_back(buffer);
        // OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
    } else
    {
        OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
    }

    // OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
}

bool BufferManager::readyToRun()
{
    dropCache();
    OS_LOGD("bufferSize: %p, %d", buffer.data(), total_sz);
    int cnt = 0;
    if (1)
    {
        //
        PthreadMutex::Writelock _l(availabe_buffers_lock);
        //
        int i = 0;
        for (auto &e : availabe_buffers)
        {
            ++i;
            auto tagstr = std::string("a") + std::to_string(i);
            e.dump(tagstr);
        }
        cnt += i;
    }
    if (1)
    {
        //
        PthreadMutex::Writelock _l(used_buffers_lock);
        //
        int i = 0;
        for (auto &e : used_buffers)
        {
            ++i;
            auto tagstr = std::string("u") + std::to_string(i);
            e.dump(tagstr);
        }
        cnt += i;
    }
    OS_LOGD("cnt=%d, total_sz=%d, slice_sz=%d, expect=%d", cnt, total_sz, slice_sz, slice_sz ? total_sz / slice_sz : 0);
    return true;
}

bool BufferManager::threadLoop(void)
{
    msleep_sliced(cb_duration_ms);
    if (exitPending())
    {
        return false;
    }
    //
    do
    {
        auto data = popData();
        if (!data.isValid())
        {
            break;
        }
        EventMarker ev(1000);
        if (cb)
        {
            cb(cb_data, data);
        }
        ReleaseBuffer(data);
        if (ev.timeout())
        {
            //
            auto timelapse_ms = ev.timelapse_ms();
            OS_LOGI("timeout: %p, take %" PRIu64 " ms", cb, timelapse_ms);
        }

    } while (!exitPending());

    return true;
}
