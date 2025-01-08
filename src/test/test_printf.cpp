#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
//
#include "cpp_helper/cpphelper_debug.hpp"
#include "manager/test_manager.h"
#include "map-macro/map.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"

static int test_printf(int reason, void* userdata)
{
    for (uint32_t i = 0; i < 64; ++i)
    {
        OS_LOGD("%.2u %02u", i, i);
    }
    return 0;
}

REG_TEST_FUNC(test_printf, test_printf, NULL)

class TypeIdDumper
{
   private:
    bool b;

    std::string       cppstr;
    const std::string cppstr0 = "hello world!";

    const char*          c_pc;
    char*                pc;
    char                 ac[16];
    const unsigned char* c_puc;
    unsigned char*       puc;
    unsigned char        auc[16];
    char                 c;
    //
    signed char   sc;
    unsigned char uc;
    int8_t        i8;
    uint8_t       u8;

    short          s;
    signed short   ss;
    unsigned short us;
    int16_t        i16;
    uint16_t       u16;

    int          i;
    signed int   si;
    int32_t      i32;
    unsigned int ui;
    uint32_t     u32;

    long          l;
    signed long   sl;
    int64_t       i64;
    unsigned long ul;
    uint64_t      u64;

    long long          ll;
    signed long long   sll;
    unsigned long long ull;

   public:
    TypeIdDumper()
    {
        reset();
    }

    void reset()
    {
        b = true;

        cppstr = "there?";

        c_pc = "hi";
        pc   = ac;
        snprintf(ac, sizeof(ac), "here!");
        c_puc = (const unsigned char*)"hi";
        puc   = auc;
        snprintf((char*)auc, sizeof(auc), "here!");
        c  = '1';
        sc = '1';
        uc = '1';
        i8 = '1';
        u8 = '1';

        s   = '1';
        ss  = '1';
        us  = '1';
        i16 = '1';
        u16 = '1';

        i   = '1';
        si  = '1';
        i32 = '1';
        ui  = '1';
        u32 = '1';

        l   = '1';
        sl  = '1';
        i64 = '1';
        ul  = '1';
        u64 = '1';

        ll  = '1';
        sll = '1';
        ull = '1';
    }


    void dumpID()
    {
#define DUMP_TYPEID(var) OS_LOGD("typeid(" #var ").name()=%s", typeid(var).name());

        DUMP_TYPEID(b);

        DUMP_TYPEID(cppstr);
        DUMP_TYPEID(cppstr0);
        OS_LOGD("typeid(char*)=%s", typeid(char*).name());
        OS_LOGD("typeid(const char*)=%s", typeid(const char*).name());
        OS_LOGD("typeid(char const *)=%s", typeid(char const*).name());
        OS_LOGD("typeid(char[])=%s", typeid(char[]).name());
        OS_LOGD("typeid(const char[])=%s", typeid(const char[]).name());
        OS_LOGD("typeid(\"\")=%s", typeid("").name());
        DUMP_TYPEID(c_pc);
        DUMP_TYPEID(pc);
        DUMP_TYPEID(ac);
        DUMP_TYPEID(c_puc);
        DUMP_TYPEID(puc);
        DUMP_TYPEID(auc);
        DUMP_TYPEID(c);
        DUMP_TYPEID(sc);
        DUMP_TYPEID(uc);
        DUMP_TYPEID(i8);
        DUMP_TYPEID(u8);

        DUMP_TYPEID(s);
        DUMP_TYPEID(ss);
        DUMP_TYPEID(us);
        DUMP_TYPEID(i16);
        DUMP_TYPEID(u16);

        DUMP_TYPEID(i);
        DUMP_TYPEID(si);
        DUMP_TYPEID(i32);
        DUMP_TYPEID(ui);
        DUMP_TYPEID(u32);

        DUMP_TYPEID(l);
        DUMP_TYPEID(sl);
        DUMP_TYPEID(i64);
        DUMP_TYPEID(ul);
        DUMP_TYPEID(u64);

        DUMP_TYPEID(ll);
        DUMP_TYPEID(sll);
        DUMP_TYPEID(ull);
    }

    std::string dumpField(const std::string& prefix = "", const std::string& surfix = "\n")
    {
        char buf[1024];
        //
        static char    testbuf1[16];
        static int16_t testbuf2[16];
        static int32_t testbuf4[16];
        //
        memset(buf, 0, sizeof(buf));
        size_t bufsz = sizeof(buf) - 1;
        //
        int offset = 0;
        //
        OS_LOGD("buf: %p,%p", buf, &buf);
        OS_LOGD("testbuf1: %p,%p, %p,%p", testbuf1, testbuf1 + 1, &testbuf1, &testbuf1 + 1);
        OS_LOGD("testbuf2: %p,%p, %p,%p", testbuf2, testbuf2 + 1, &testbuf2, &testbuf2 + 1);
        OS_LOGD("testbuf4: %p,%p, %p,%p", testbuf4, testbuf4 + 1, &testbuf4, &testbuf4 + 1);


#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat"
#endif
#define TMP_SNPRINTF_STRUCTURE_FIELD(field) SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, field);
        MAP(TMP_SNPRINTF_STRUCTURE_FIELD,
            b,
            cppstr,
            cppstr0,
            c_pc,
            pc,
            ac,
            c,
            sc,
            uc,
            i8,
            u8,
            s,
            ss,
            us,
            i16,
            i,
            si,
            i32,
            ui,
            u32,
            l,
            sl,
            i64,
            ul,
            u64,
            ll,
            sll,
            ull);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, b);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, cppstr);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, cppstr0);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, c_pc);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, pc);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, ac);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, c);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, sc);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, uc);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, i8);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, u8);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, s);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, ss);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, us);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, i16);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, u16);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, i);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, si);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, i32);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, ui);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, u32);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, l);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, sl);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, i64);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, ul);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, u64);

        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, ll);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, sll);
        // SNPRINTF_STRUCTURE_FIELD(buf, bufsz, offset, prefix.c_str(), surfix.c_str(), this, ull);

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

        return buf;
    }
};

static int test_stdump(int reason, void* userdata)
{
    TypeIdDumper obj;
    //
    obj.dumpID();
    //
    OS_LOGD("%s", obj.dumpField().c_str());

    return 0;
}

REG_TEST_FUNC(test_stdump, test_stdump, NULL)
