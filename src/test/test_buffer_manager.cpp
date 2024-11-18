#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include "utils/buffer_manager.hpp"

//
#include "test.h"
#include "utils/os_tools.h"

#define ITEM_DATA_LEN  (1020)
#define CB_DURATION_MS (1000)

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

static BufferManager m_buffer(sizeof(uint32_t) + ITEM_DATA_LEN, 32, MyDataItem_handler, NULL, CB_DURATION_MS);

int test_buffer_manager(void *userdata)
{
    std::string test_string = "hello";
    //
    m_buffer.tryRun();
    //
    OS_LOGD("buffer manager test begin.");
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
    //
    OS_LOGD("buffer manager wait.");
    //
    usleep(1000 * CB_DURATION_MS * 2);
    //
    OS_LOGD("buffer manager end.");

    return 0;
}

REG_TEST_FUNC(buffer_manager, test_buffer_manager, NULL)
