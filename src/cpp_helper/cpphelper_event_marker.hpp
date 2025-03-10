#ifndef __CPPHELPER_EVENT_MARKER_H__
#define __CPPHELPER_EVENT_MARKER_H__


#include <time.h>
#include <cstdint>


class EventMarker
{
   private:
    //
    uint64_t timeout_ms = 2000;
    //
    uint64_t touched_ts = 0;

   public:
    EventMarker(uint64_t timeout_ms = 2000) : timeout_ms(timeout_ms)
    {
        touch();
    }

    uint64_t get_timeout_thr()
    {
        return timeout_ms;
    }

    // current timestamp in milisecond
    static uint64_t timestamp_ms()
    {
        struct timespec monotime;
        clock_gettime(CLOCK_MONOTONIC, &monotime);
        return (uint64_t)monotime.tv_sec * 1000 + monotime.tv_nsec / 1000 / 1000;
    }

    // there is an event, mark it by touching
    void touch()
    {
        touched_ts = timestamp_ms();
    }

    // timelapse compare to given timestamp
    static uint64_t timelapse_ms(uint64_t cmpTs)
    {
        auto curts = timestamp_ms();
        //
        auto lapse_ms = curts;
        //
        if (curts >= cmpTs)
        {
            lapse_ms = curts - cmpTs;
        } else
        {
            // overflow: UINT64_MAX + 1 + curts - cmpTs = UINT64_MAX - ( cmpTs - 1 - curts )
            lapse_ms = UINT64_MAX - (cmpTs - 1 - curts);
        }
        // OS_LOGD("curts=%" PRIu64 ", cmpTs=%" PRIu64 ", lapse_ms=%" PRIu64 "", curts, cmpTs, lapse_ms);
        return lapse_ms;
    }

    // get timelapse in milisecond since last touching
    uint64_t timelapse_ms()
    {
        return timelapse_ms(touched_ts);
    }

    /**
     * @brief time left before timeout
     *
     */
    uint64_t left_ms()
    {
        auto t = timelapse_ms();
        //
        if (t >= timeout_ms)
        {
            return 0;
        }
        return timeout_ms - t;
    }

    /**
     * @brief it is timeout since last touching
     *
     * @return true
     * @return false
     */
    bool timeout()
    {
        return left_ms() ? false : true;
    }
};


#endif  // __CPPHELPER_EVENT_MARKER_H__
