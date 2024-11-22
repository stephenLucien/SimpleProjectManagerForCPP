#ifndef __TEST_MANAGER_H__
#define __TEST_MANAGER_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef int (*TestFunc)(int reason, void* userdata);

int reg_test(const char* tag, TestFunc func, void* data);

int unreg_test(const char* tag);

int run_test(const char* tag, int reason = 0);


#define REG_TEST_FUNC(tag, func, data)                             \
    __attribute__((constructor)) static void reg_test_func_##tag() \
    {                                                              \
        reg_test(#tag, func, (void*)data);                         \
    }


#ifdef __cplusplus
}
#endif


#endif  // __TEST_MANAGER_H__
