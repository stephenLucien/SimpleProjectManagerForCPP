#ifndef __VALUE_TRACKER_H__
#define __VALUE_TRACKER_H__


#include "pthread_class.hpp"
#include "pthread_mutex.hpp"

//
#include <cstdint>
#include <cstring>
#include <unordered_map>


class ValueTracker : public PthreadWrapper
{
   public:
    typedef void (*OnValueChangedHdl)(void* ptr);

   private:
    typedef struct
    {
        uint32_t sz;
        uint32_t final_timeout_ms;
        uint8_t  buf[8];
    } ValueItem;

    typedef struct
    {
        //
        uint64_t last_touch_ts;
        //
        int32_t need_handled;
        //
        OnValueChangedHdl hdl;
        //
        ValueItem item;
    } TrackItem;

    //
    PthreadMutex mtx;
    //
    std::unordered_map<void*, TrackItem> track_list;
    //
    int period_ms;

    ValueTracker()
    {
        //
        period_ms = 33;
    }
    ~ValueTracker()
    {
    }

    static inline uint64_t get_ts()
    {
        struct timespec monotime;
        clock_gettime(CLOCK_MONOTONIC, &monotime);
        uint64_t ts;
        ts = (uint64_t)monotime.tv_sec * 1000 + monotime.tv_nsec / 1000 / 1000;
        return ts;
    }

    bool threadLoop()
    {
        usleep(1000 * period_ms);

        PthreadMutex::Readlock lock(mtx);
        for (auto& e : track_list)
        {
            if (exitPending())
            {
                return false;
            }
            auto ptr0 = e.first;
            auto ptr1 = (void*)e.second.item.buf;
            //
            auto sz = e.second.item.sz;
            //
            auto curts = get_ts();
            if (memcmp(ptr0, ptr1, sz) != 0)
            {
                memcpy(ptr1, ptr0, sz);
                e.second.last_touch_ts = curts;
                //
                e.second.need_handled = 1;
            }
            if (e.second.need_handled && curts >= e.second.last_touch_ts + e.second.item.final_timeout_ms)
            {
                if (e.second.hdl)
                {
                    e.second.hdl(ptr0);
                }
                e.second.need_handled = 0;
            }
        }

        return true;
    }

   public:
    void addTrack(void* ptr, uint32_t sz, OnValueChangedHdl hdl, uint32_t wait_for_final_ms = 0)
    {
        PthreadMutex::Writelock lock(mtx);
        track_list[ptr] = {0};
        //
        track_list[ptr].hdl = hdl;
        //
        track_list[ptr].item.sz = sz;
        //
        memcpy(track_list[ptr].item.buf, ptr, sz);
        //
        track_list[ptr].item.final_timeout_ms = wait_for_final_ms;
    }

    void rmTrack(void* ptr)
    {
        PthreadMutex::Writelock lock(mtx);
        track_list.erase(ptr);
    }

    static ValueTracker* getInstance()
    {
        static ValueTracker obj;
        if (!obj.isRunning())
        {
            obj.run();
        }
        return &obj;
    }
};

#define EASY_TRACK_VALUE(refvalue, wait_for_final_ms, codeblock)                                                          \
    static void on_##refvalue##_changed(void* ptr)                                                                        \
    {                                                                                                                     \
        auto pvalue = (__typeof__(refvalue)*)ptr;                                                                         \
        auto value  = *pvalue;                                                                                            \
        {                                                                                                                 \
            codeblock                                                                                                     \
        }                                                                                                                 \
    }                                                                                                                     \
    __attribute__((constructor)) static void RegValueTacker_##refvalue()                                                  \
    {                                                                                                                     \
        ValueTracker::getInstance()->addTrack(&(refvalue), sizeof(refvalue), on_##refvalue##_changed, wait_for_final_ms); \
    }


#endif  // __VALUE_TRACKER_H__
