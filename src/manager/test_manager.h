#ifndef __TEST_MANAGER_H__
#define __TEST_MANAGER_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef int (*TestFunc)(int reason, void* userdata);

int reg_test(const char* tag, TestFunc func, void* data);

int unreg_test(const char* tag);

int run_test(const char* tag, int reason = 0);

int run_tests(int reason = 0);

#define REG_TEST_FUNC(tag, func, data)                             \
    __attribute__((constructor)) static void reg_test_func_##tag() \
    {                                                              \
        reg_test(#tag, func, (void*)data);                         \
    }


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
    #include <string>
    #include <vector>

int run_tests(const std::vector<std::string>& tests, int reason);

int run_tests(const char* tests, int reason);

#endif

#endif  // __TEST_MANAGER_H__
