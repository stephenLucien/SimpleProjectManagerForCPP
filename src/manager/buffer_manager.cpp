#include "buffer_manager.hpp"

//
#include <cstdint>
#include <list>
#include <vector>

//
#include "utils/os_tools.h"
#include "utils/pthread_class.hpp"

BufferManager::BufferManager(int slice_sz, int total_cnt, BufferItemHDL func, void *func_data, int cb_duration_ms)
{
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

    for (int pos = 0; pos + slice_sz <= total_sz; pos += slice_sz)
    {
        availabe_buffers.emplace_back(BufferItem(buffer.data() + pos, slice_sz));
    }
    OS_LOGV("BufferItem count: %zu, slice_sz: %d", availabe_buffers.size(), slice_sz);
#if 0
        for (auto &e : availabe_buffers)
        {
            OS_LOGV("BufferItem: %p, %d", e.buf, e.bufsz);
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


BufferManager::BufferItem BufferManager::getBuffer()
{
    BufferItem buffer;
    //
    PthreadMutex::Writelock _l(availabe_buffers_lock);
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
    used_buffers.push_back(buffer);

    // OS_LOGV("%s %p", __PRETTY_FUNCTION__, buffer.buf);
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
    availabe_buffers.push_back(buffer);

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
        // OS_LOGV("%p begin", cb);
        // auto bts = os_get_timestamp_ms();
        if (cb)
        {
            cb(cb_data, data);
        }
        // auto ets = os_get_timestamp_ms();
        // OS_LOGV("%p, take %llu ms", cb, ets - bts);

        ReleaseBuffer(data);
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
