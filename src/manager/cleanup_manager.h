#ifndef MANAGER_CLEANUP_MANAGER_H_
#define MANAGER_CLEANUP_MANAGER_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef int (*CleanupFunc)(int reason, void* userdata);

int reg_cleanup(const char* tag, CleanupFunc func, void* data);

int unreg_cleanup(const char* tag);

int run_cleanup(const char* tag, int reason = 0);


#define REG_CLEANUP_FUNC(tag, func, data)                             \
    __attribute__((constructor)) static void reg_cleanup_func_##tag() \
    {                                                                 \
        reg_cleanup(#tag, func, (void*)data);                         \
    }


#ifdef __cplusplus
}
#endif


#endif  // MANAGER_CLEANUP_MANAGER_H_
