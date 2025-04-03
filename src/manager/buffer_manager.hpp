#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__

//
#include <cstdlib>
#include <list>
#include <vector>

//
#include "cpp_helper/cpphelper_pthread.hpp"

class BufferManager : public PthreadWrapper
{
   public:
    class BufferItem
    {
       private:
        //
        void *pool = NULL;
        //
        bool isCopy = false;
        //
        void init(void *buf = NULL, int bufsz = 0, bool b_copy = false, void *pool = NULL);
        //
        void deinit();

       public:
        //
        BufferItem(void *buf = NULL, int bufsz = 0, bool b_copy = false, void *pool = NULL);
        //
        virtual ~BufferItem();
        //
        void *getPool();
        //
        void dump();

        bool isValid();
        //
        void *buf = NULL;
        //
        int bufsz = 0;
    };


    typedef void (*BufferItemHDL)(void *userdata, BufferItem buffer);

   public:
    BufferManager(int slice_sz = 4096, int total_cnt = 20, BufferItemHDL func = NULL, void *func_data = NULL, int cb_duration_ms = 20);
    virtual ~BufferManager();

    void clear();

    BufferItem getBuffer(int timeout_ms = -1);

    void pushData(BufferItem buffer);



    BufferItem popData();

    void ReleaseBuffer(BufferItem buffer);


    bool tryRun();

   private:
    std::vector<uint8_t> buffer;
    //
    int total_sz;
    int slice_sz;
    //
    std::list<BufferItem> availabe_buffers;
    //
    PthreadMutex availabe_buffers_lock;

    //
    std::list<BufferItem> used_buffers;
    //
    PthreadMutex used_buffers_lock;
    //
    BufferItemHDL cb;
    //
    void *cb_data;
    //
    int cb_duration_ms;
    //
    void _clearUsed();
    //
    void _genBuffers();
    //
    bool threadLoop(void) override;
};

#endif  // __BUFFER_MANAGER_H__
