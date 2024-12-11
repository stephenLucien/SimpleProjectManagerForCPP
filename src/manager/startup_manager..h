#ifndef MANAGER_STARTUP_MANAGER_H_
#define MANAGER_STARTUP_MANAGER_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef int (*StartupFunc)(int reason, void* userdata);

int reg_startup(const char* tag, StartupFunc func, void* data);

int unreg_startup(const char* tag);

int run_startup(const char* tag, int reason = 0);


#define REG_STARTUP_FUNC(tag, func, data)                             \
    __attribute__((constructor)) static void reg_startup_func_##tag() \
    {                                                                 \
        reg_startup(#tag, func, (void*)data);                         \
    }


#ifdef __cplusplus
}
#endif


#endif  // MANAGER_STARTUP_MANAGER_H_
