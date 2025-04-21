#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
//
#include "cpp_helper/cpphelper_cJSON.hpp"
#include "cpp_helper/cpphelper_os.hpp"
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

namespace TestCppClass
{
class A
{
   private:
    void* ptr;

   public:
    A()
    {
        OS_LOGD("");
        ptr = malloc(8);
    }
    ~A()
    {
        OS_LOGD("");
        if (ptr)
        {
            OS_LOGD("");
            free(ptr);
        }
    }
};

class Ac
{
   private:
    void* ptr;

   public:
    Ac()
    {
        OS_LOGD("");
        ptr = malloc(8);
    }
    virtual ~Ac()
    {
        OS_LOGD("");
        if (ptr)
        {
            OS_LOGD("");
            free(ptr);
        }
    }
};

class B : public A
{
   private:
    void* ptr_b;

   public:
    B()
    {
        OS_LOGD("");
        ptr_b = malloc(8);
    }
    ~B()
    {
        OS_LOGD("");
        if (ptr_b)
        {
            OS_LOGD("");
            free(ptr_b);
        }
    }
};


class Bc : public Ac
{
   private:
    void* ptr_b;

   public:
    Bc()
    {
        OS_LOGD("");
        ptr_b = malloc(8);
    }
    virtual ~Bc()
    {
        OS_LOGD("");
        if (ptr_b)
        {
            OS_LOGD("");
            free(ptr_b);
        }
    }
};


class Bd : public Ac
{
   private:
    void* ptr_d;

   public:
    Bd()
    {
        OS_LOGD("");
        ptr_d = malloc(8);
    }
    ~Bd()
    {
        OS_LOGD("");
        if (ptr_d)
        {
            OS_LOGD("");
            free(ptr_d);
        }
    }
};

class Cc : public Bc
{
   private:
    void* ptr_c;

   public:
    Cc()
    {
        OS_LOGD("");
        ptr_c = malloc(8);
    }
    virtual ~Cc()
    {
        OS_LOGD("");
        if (ptr_c)
        {
            OS_LOGD("");
            free(ptr_c);
        }
    }
};


class Cd : public Bd
{
   private:
    void* ptr_c;

   public:
    Cd()
    {
        OS_LOGD("");
        ptr_c = malloc(8);
    }
    ~Cd()
    {
        OS_LOGD("");
        if (ptr_c)
        {
            OS_LOGD("");
            free(ptr_c);
        }
    }
};

}  // namespace TestCppClass

static int test_virtualDestructor(int reason, void* userdata)
{
    auto pb = new TestCppClass::B();
    //
    auto pb_a = (TestCppClass::A*)pb;
    //
    delete pb_a;


    return 0;
}

REG_TEST_FUNC(test_virtualDestructor, test_virtualDestructor, NULL)

static int test_virtualDestructor1(int reason, void* userdata)
{
    auto pbc = new TestCppClass::Bc();
    //
    auto pbc_ac = (TestCppClass::Ac*)pbc;
    //
    delete pbc_ac;


    return 0;
}

REG_TEST_FUNC(test_virtualDestructor1, test_virtualDestructor1, NULL)

static int test_virtualDestructor2(int reason, void* userdata)
{
    auto pbc = new TestCppClass::Bd();
    //
    auto pbc_ac = (TestCppClass::Ac*)pbc;
    //
    delete pbc_ac;


    return 0;
}

REG_TEST_FUNC(test_virtualDestructor2, test_virtualDestructor2, NULL)


static int test_virtualDestructor3(int reason, void* userdata)
{
    auto pbc = new TestCppClass::Cc();
    //
    auto pbc_ac = (TestCppClass::Ac*)pbc;
    //
    delete pbc_ac;


    return 0;
}

REG_TEST_FUNC(test_virtualDestructor3, test_virtualDestructor3, NULL)

static int test_virtualDestructor4(int reason, void* userdata)
{
    auto pbc = new TestCppClass::Cd();
    //
    auto pbc_ac = (TestCppClass::Ac*)pbc;
    //
    delete pbc_ac;


    return 0;
}

REG_TEST_FUNC(test_virtualDestructor4, test_virtualDestructor4, NULL)
