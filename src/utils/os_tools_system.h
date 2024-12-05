#ifndef OS_TOOLS_SYSTEM_H
#define OS_TOOLS_SYSTEM_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int os_running_loop();

int system_wrap(char *buf, size_t bufsz, const char *msg, ...);

#define SYSTEM(msg, ...) system_wrap((char *)__builtin_alloca(256), 256, msg, ##__VA_ARGS__)

int is_sudo(int dump = 0);

int os_setup_backtrace();

void cleanup_on_exit_impl(int sig_num);

int os_setup_exit();


/**
 * @brief write data to file
 *
 * @param fn
 * @param data
 * @param datalen
 * @return int
 * - negative : write fail
 * - other : bytes written
 */
int write_data_to_file(const char *fn, const char *data, size_t datalen);

/**
 * @brief read data from file
 *
 * @param filename
 * @param buf
 * @param bufsz
 * @param read_cnt
 * - negative: err
 * - other : bytes read
 * @return char*
 */
char *read_data_from_file(const char *fn, char *buf, size_t bufsz, int *read_cnt);


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

    #include <string>

static inline int write_text_file(const std::string &path, const std::string &str)
{
    return write_data_to_file(path.c_str(), str.c_str(), str.length());
}

static inline std::string read_text_file(const std::string &path, const std::string &def = std::string())
{
    int read_cnt = -1;
    //
    std::string res = read_data_from_file(path.c_str(), (char *)__builtin_alloca(4096), 4096, &read_cnt);
    if (read_cnt >= 0)
    {
        return res;
    }
    return def;
}

#endif



#endif /* OS_TOOLS_SYSTEM_H */
