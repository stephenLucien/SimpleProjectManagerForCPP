#include "os_tools_system.h"

//
#include "os_tools_log.h"

//
#include <cxxabi.h>
#include <dirent.h>
#include <execinfo.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <unordered_map>

//
#include "cpp_helper/cpphelper_os.hpp"
#include "cstring_proc.h"
#include "pthread_class.hpp"

//
int m_app_running = 1;
//
static int m_return_code = EXIT_SUCCESS;



void __attribute__((weak)) os_init_on_startup_impl(int num)
{
    OS_LOGV("%s", __FILE__);
}


void __attribute__((weak)) os_deinit_on_exit_impl(int sig_num)
{
    OS_LOGV("%s", __FILE__);
}

//
void __attribute__((weak)) os_running_loop_impl()
{
    OS_LOGV("%s", __FILE__);
    while (m_app_running)
    {
        sleep(1);
    }
    OS_LOGV("%s", __FILE__);
}

void __attribute__((weak)) os_stop_running_loop_impl(int code)
{
    OS_LOGV("%s", __FILE__);
}


void os_init_on_startup(int num)
{
    //
    os_setup_backtrace();
    os_setup_exit();

    //
    os_init_on_startup_impl(num);
}

int os_running_loop()
{
    //
    os_running_loop_impl();
    //
    os_deinit_on_exit_impl(0);

    return m_return_code;
}


void os_stop_running_loop(int code)
{
    os_stop_running_loop_impl(code);
    //
    m_return_code = code;
    //
    m_app_running = 0;
}


int system_wrap(char *buf, size_t bufsz, const char *msg, ...)
{
    int ret = -1;
    //
    va_list ap;
    va_start(ap, msg);
    //
    int wc = vsnprintf(buf, bufsz, msg, ap);
    if (wc > 0)
    {
        ret = system(buf);
    }
    va_end(ap);
    return ret;
}

int is_sudo(int dump)
{
    auto uid  = getuid();
    auto euid = geteuid();

    if (uid == 0)
    {
        if (dump)
        {
            OS_LOGV("root");
        }
        return 1;
    }
    if (euid == 0)
    {
        if (dump)
        {
            OS_LOGV("sudo");
        }
        return 1;
    }
    if (dump)
    {
        OS_LOGV("uid: %u", uid);
        OS_LOGV("euid: %u", euid);
    }
    return 0;
}


static void dump_backtrace(int sig_num, siginfo_t *info, void *ucontext)
{
    void  *array[50];
    char **messages;
    int    size, i;
    //
    OS_PRINT("signum: %d (%s)", sig_num, strsignal(sig_num));

    size = backtrace(array, 50);

    messages = backtrace_symbols(array, size);

    /* skip first stack frame (points here) */
    for (i = 1; i < size && messages != NULL; ++i)
    {
        int status;
        //
        char *realname = abi::__cxa_demangle(messages[i], NULL, NULL, &status);
        OS_LOGE("[bt]: (%d) %s", i, realname ? realname : messages[i] ? messages[i] : "nil");
        free(realname);
    }

    free(messages);

    //
    os_stop_running_loop(EXIT_FAILURE);
}

static inline int setup_backtrace_signal_hdl(int sig_num)
{
    int ret = 0;
    //
    struct sigaction sigact;

    sigact.sa_sigaction = dump_backtrace;
    //
    sigact.sa_flags = SA_RESETHAND;

    if (sigaction(sig_num, &sigact, (struct sigaction *)NULL) != 0)
    {
        OS_PRINT("error setting signal handler for %d (%s)\n", sig_num, strsignal(sig_num));

        ret = -1;
    }
    return ret;
}

int os_setup_backtrace()
{
    int ret = 0;

    if (setup_backtrace_signal_hdl(SIGSEGV))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGBUS))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGFPE))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGABRT))
    {
        ret = -1;
    }
    if (setup_backtrace_signal_hdl(SIGILL))
    {
        ret = -1;
    }
    return ret;
}


static void on_exit_signal(int sig_num, siginfo_t *info, void *ucontext)
{
    //
    OS_LOGW("signum: %d (%s)", sig_num, strsignal(sig_num));

    //
    os_stop_running_loop(EXIT_SUCCESS);
}

static inline int setup_exit_signal_hdl(int sig_num)
{
    int ret = 0;
    //
    struct sigaction sigact;

    sigact.sa_sigaction = on_exit_signal;
    //
    sigact.sa_flags = SA_RESETHAND;

    if (sigaction(sig_num, &sigact, (struct sigaction *)NULL) != 0)
    {
        OS_PRINT("error setting signal handler for %d (%s)\n", sig_num, strsignal(sig_num));

        ret = -1;
    }
    return ret;
}

int os_setup_exit()
{
    int ret = 0;

    if (setup_exit_signal_hdl(SIGINT))
    {
        ret = -1;
    }
#if 0
    // Will always fail, SIGKILL is intended to force kill your process
    if (setup_exit_signal_hdl(SIGKILL))
    {
        // You can never handle SIGKILL anyway...
        ret = -1;
    }
#endif
    return ret;
}


int write_data_to_file(const char *fn, const char *data, size_t datalen)
{
    int ret = -1;
    //
    if (!data || datalen <= 0)
    {
        return ret;
    }
    //
    CPP_FOPEN(fd, fn, "w");
    if (!fd)
    {
        OS_PRINT("open file fail: %s", fn);
        return ret;
    }
    size_t wc = fwrite(data, 1, datalen, fd);
    if (wc != datalen)
    {
        OS_PRINT("write len(%zu/%zu) %s", wc, datalen, fn);
    }

    return (int)wc;
}

char *read_data_from_file(const char *fn, char *buf, size_t bufsz, int *read_cnt)
{
    if (read_cnt)
    {
        *read_cnt = -1;
    }
    if (!buf || bufsz <= 0)
    {
        return buf;
    }
    //
    memset(buf, 0, bufsz);
    //
    CPP_FOPEN(fd, fn, "r");
    if (!fd)
    {
        OS_PRINT("open file fail: %s", fn);
        return buf;
    }
    do
    {
#if 0
        if (fseek(fd, 0, SEEK_END) != 0)
        {
            OS_PRINT("fseek end fail:%s", fn);
            break;
        }
        auto sz = ftell(fd);
        if (sz < 0)
        {
            OS_PRINT("ftell fail:%s", fn);
            break;
        }
        if (fseek(fd, 0, SEEK_SET) != 0)
        {
            OS_PRINT("fseek set fail:%s", fn);
            break;
        }
        if (sz >= bufsz)
        {
            OS_PRINT("overflow, fn:%s, max:%zu, but %ld ", fn, bufsz, sz);
            break;
        }
#else
        auto sz = bufsz - 1;
#endif

        auto rc = fread(buf, 1, sz, fd);
        if (read_cnt)
        {
            *read_cnt = (int)rc;
        }
#if 0
        if (rc == sz)
        {
        } else
        {
            OS_PRINT("read incomplete (%zu, %d): %s", rc, sz, fn);
        }
#endif

    } while (0);

    return buf;
}

//
static PthreadMutex m_meminfo_mtx;
//
static char m_meminfo_buf[2048];
//
static std::string m_meminfo_unit;
//
static std::unordered_map<std::string, size_t> m_meminfo;
/**
 * @brief
 * - sysfs: /proc/meminfo
MemTotal:          67872 kB
MemFree:            5840 kB
MemAvailable:      33592 kB
Buffers:            5312 kB
Cached:            24540 kB
SwapCached:            0 kB
Active:            19420 kB
Inactive:          21868 kB
Active(anon):      11460 kB
Inactive(anon):        8 kB
Active(file):       7960 kB
Inactive(file):    21860 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:             0 kB
SwapFree:              0 kB
Dirty:                 4 kB
Writeback:             0 kB
AnonPages:         11432 kB
Mapped:            20748 kB
Shmem:                40 kB
Slab:              12540 kB
SReclaimable:       1352 kB
SUnreclaim:        11188 kB
KernelStack:        1160 kB
PageTables:          688 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:       33936 kB
Committed_AS:     725612 kB
VmallocTotal:     909312 kB
VmallocUsed:           0 kB
VmallocChunk:          0 kB
CmaTotal:           2048 kB
CmaFree:             844 kB

 *
 */
int os_update_meminfo()
{
    int ret = -1;
    //
    auto fn = "/proc/meminfo";
    //
    CPP_FOPEN(fd, fn, "r");
    //
    auto buf = m_meminfo_buf;
    //
    auto buf_sz = sizeof(m_meminfo_buf) - 1;
    if (!buf || !fd)
    {
        return ret;
    }
    //
    PthreadMutex::Writelock lock(m_meminfo_mtx);
    //
    auto rc = fread(buf, 1, buf_sz, fd);
    if (rc <= 0)
    {
        return ret;
    }
    buf[rc] = '\0';
    // OS_LOGV("(rc=%zu):\n%s", rc, buf);
    ret = 0;
    //
    std::vector<char *> lines;
    str2tokens(buf, lines, "\n");
    for (auto &line : lines)
    {
        // OS_LOGV("%s", line);
        std::vector<char *> keyvalunit;
        str2tokens(line, keyvalunit, ": ");
        if (keyvalunit.size() == 3)
        {
            size_t sz = 0;
            if (sscanf(keyvalunit[1], "%zu", &sz) == 1)
            {
                m_meminfo[keyvalunit[0]] = sz;
                ++ret;
            }
            if (std::string(keyvalunit[2]) != m_meminfo_unit)
            {
                m_meminfo_unit = keyvalunit[2];
                OS_LOGV("meminfo unit: %s", m_meminfo_unit.c_str());
            }
        }
    }
    return ret;
}

std::string os_get_meminfo_unit()
{
    PthreadMutex::Readlock lock(m_meminfo_mtx);
    return m_meminfo_unit;
}

int os_get_meminfo(std::unordered_map<std::string, size_t> &infos, int update)
{
    //
    int ret = -1;
    if (update)
    {
        //
        ret = os_update_meminfo();
        if (ret < 0)
        {
            return ret;
        }
    }
    PthreadMutex::Readlock lock(m_meminfo_mtx);
    infos = m_meminfo;
    return 0;
}

int os_get_available_ram(int update)
{
    int ret = -1;
    if (update)
    {
        //
        ret = os_update_meminfo();
        if (ret < 0)
        {
            return ret;
        }
    }
    PthreadMutex::Readlock lock(m_meminfo_mtx);
    //
    int SReclaimable = 0;
    if (m_meminfo.find("SReclaimable") != m_meminfo.end())
    {
        SReclaimable = m_meminfo["SReclaimable"];
    }
    int Inactive_file = 0;
    if (m_meminfo.find("Inactive(file)") != m_meminfo.end())
    {
        Inactive_file = m_meminfo["Inactive(file)"];
    }
    int MemFree = 0;
    if (m_meminfo.find("MemFree") != m_meminfo.end())
    {
        MemFree = m_meminfo["MemFree"];
    }
    return SReclaimable + Inactive_file + MemFree;
}

int os_get_dir_entries(const std::string &path, std::list<std::string> &entries, int d_type)
{
    int ret = -1;
    //
    entries.clear();
    //
    CPP_OPENDIR(dir, path.c_str());
    if (!dir)
    {
        OS_LOGE("");
        return ret;
    }
    ret = 0;
    do
    {
        auto entry = readdir(dir);
        if (!entry)
        {
            break;
        }
        // OS_LOGV("type=%d,name=%s", entry->d_type, entry->d_name);
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        if (d_type < 0)
        {
            entries.push_back(entry->d_name);
        } else if (entry->d_type == d_type)
        {
            entries.push_back(entry->d_name);
        } else if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK)
        {
            std::string fp = path + std::string("/") + std::string(entry->d_name);
            //
            struct stat stbuf;
            // stat follows symlinks, lstat doesn't.
            auto tmpret = stat(fp.c_str(), &stbuf);
            if (tmpret == 0)
            {
                if (0)
                {
                } else if (d_type == DT_FIFO)
                {
                    if (S_ISFIFO(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                } else if (d_type == DT_CHR)
                {
                    if (S_ISCHR(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                } else if (d_type == DT_DIR)
                {
                    if (S_ISDIR(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                } else if (d_type == DT_BLK)
                {
                    if (S_ISBLK(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                } else if (d_type == DT_REG)
                {
                    if (S_ISREG(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                } else if (d_type == DT_SOCK)
                {
                    if (S_ISSOCK(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                }
#if 0
                else if (d_type == DT_LNK)
                {
                    if (S_ISLNK(stbuf.st_mode))
                    {
                        entries.push_back(entry->d_name);
                    }
                }
#endif
            } else
            {
                OS_LOGV("stat fail, ret=%d, file:%s", tmpret, fp.c_str());
            }
        }
    } while (true);

    return entries.size();
}
