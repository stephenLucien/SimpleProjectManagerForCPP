#include <unistd.h>
#include <cstdint>
#include <cstdio>

//
#include "manager/buffer_manager.hpp"
#include "manager/cleanup_manager.h"
#include "manager/test_manager.h"
#include "utils/os_tools.h"

#define ITEM_DATA_LEN  (1020)
#define CB_DURATION_MS (1000)

namespace
{
class MyDataItem
{
   public:
    MyDataItem(BufferManager::BufferItem buffer = {0})
    {
        setBuf(buffer);
    }
    void setBuf(BufferManager::BufferItem &buffer)
    {
        //
        this->buffer = buffer;
        //
        pdatalen = NULL;
        pdata    = NULL;
        //
        if (!buffer.isValid())
        {
            return;
        }
        //
        uint8_t *ptr    = (uint8_t *)buffer.buf;
        int      offset = 0;
        //
        pdatalen  = (uint32_t *)(ptr + offset);
        offset   += sizeof(pdatalen[0]);
        //
        pdata = (uint8_t *)(ptr + offset);
    }
    bool isValid()
    {
        return pdatalen && pdata;
    }

    uint32_t *pdatalen;
    uint8_t  *pdata;
    //
    BufferManager::BufferItem buffer;
};

static void MyDataItem_handler(void *userdata, BufferManager::BufferItem buffer)
{
    MyDataItem data(buffer);
    if (!data.isValid())
    {
        return;
    }

    OS_PRINT("len=%lu, data:%s", *data.pdatalen, (char *)data.pdata);
}
//
static BufferManager m_buffer(sizeof(uint32_t) + ITEM_DATA_LEN, 32, MyDataItem_handler, NULL, CB_DURATION_MS);

class BufferTester : public PthreadWrapper
{
   private:
    bool readyToRun() override
    {
        return true;
    };
    bool threadLoop() override
    {
        msleep_sliced(CB_DURATION_MS * 4);
        if (exitPending())
        {
            return false;
        }
        push(std::to_string(rand()));
        return true;
    }

   public:
    BufferTester()
    {
        setTaskName("BufferTester");
    }
    static int push(const std::string &test_string)
    {
        //
        auto buf = m_buffer.getBuffer();
        if (!buf.isValid())
        {
            return -1;
        }
        MyDataItem data(buf);
        if (data.isValid())
        {
            snprintf((char *)data.pdata, ITEM_DATA_LEN, "%s", test_string.c_str());
            *data.pdatalen = test_string.length();
        }
        m_buffer.pushData(buf);
        return 0;
    }
};
//
static BufferTester m_tester;

}  // namespace

static int buffertest_end(int num, void *data)
{
    if (!(m_tester.isRunning() || m_buffer.isRunning()))
    {
        return 0;
    }
    //
    OS_LOGD("buffer manager test end0.");
    m_tester.requestExitAndWait();
    m_buffer.requestExitAndWait();
    //
    OS_LOGD("buffer manager test end1.");
    return 0;
}
REG_CLEANUP_FUNC(buffertest_end, buffertest_end, NULL)

static int test_buffer_manager(int reason, void *userdata)
{
    //
    OS_LOGD("buffer manager test begin.");
    //
    m_buffer.tryRun();
    //
    m_tester.tryRun();

    return 0;
}

REG_TEST_FUNC(buffer_manager, test_buffer_manager, NULL)
