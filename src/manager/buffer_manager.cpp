#include "buffer_manager.hpp"

//
#include <cstdint>
#include <list>
#include <vector>

//
#include "cpp_helper/cpphelper_event_marker.hpp"
#include "cpp_helper/cpphelper_pthread.hpp"
#include "utils/os_tools.h"



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
void BufferManager::BufferItem::dump()
{
    OS_LOGV("[p:%p%s] ptr: %p, sz: %d", pool, isCopy ? ",copied" : "", buf, bufsz);
}



BufferManager::BufferManager(int slice_sz, int total_cnt, BufferItemHDL func, void *func_data, int cb_duration_ms)
{
    setTaskName("BufferManager");
    //
    this->slice_sz = slice_sz;
    this->total_sz = slice_sz * total_cnt;
    //
    this->cb = func;
    //
    this->cb_data = func_data;
    //
    this->cb_duration_ms = cb_duration_ms;
}

BufferManager::~BufferManager()
{
    {
        PthreadMutex::Writelock _l(availabe_buffers_lock);
        availabe_buffers.clear();
    }
    {
        PthreadMutex::Writelock _l(used_buffers_lock);
        used_buffers.clear();
    }

    buffer.clear();
}

void BufferManager::_clearUsed()
{
    used_buffers.clear();
}

void BufferManager::_genBuffers()
{
    buffer.resize(total_sz);

    OS_LOGV("bufferSize: %p, %d", buffer.data(), total_sz);
    availabe_buffers.clear();
    for (int pos = 0; pos + slice_sz <= total_sz; pos += slice_sz)
    {
        availabe_buffers.emplace_back(BufferItem(buffer.data() + pos, slice_sz, false, buffer.data()));
    }
    OS_LOGV("BufferItem count: %zu, slice_sz: %d", availabe_buffers.size(), slice_sz);
#if 0
    for (auto &e : availabe_buffers)
    {
        e.dump();
    }
#endif
}

void BufferManager::clear()
{
    {
        PthreadMutex::Writelock _l(used_buffers_lock);
        _clearUsed();
    }
    {
        PthreadMutex::Writelock _l(availabe_buffers_lock);
        _genBuffers();
    }
}


BufferManager::BufferItem BufferManager::getBuffer(int timeout_ms)
{
    BufferItem buffer;
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

bool BufferManager::threadLoop(void)
{
    sleep(cb_duration_ms);
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


bool BufferManager::tryRun()
{
    bool ret = false;

    if (isRunning())
    {
        return ret;
    }

    clear();
    return run();
}
