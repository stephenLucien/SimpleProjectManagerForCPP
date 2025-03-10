#ifndef __CPPHELPER_SQLITE3_H__
#define __CPPHELPER_SQLITE3_H__

#include <sqlite3.h>
#include <string>

//
#include "utils/os_tools_log.h"
#include "cpp_helper/cpphelper_pthread.hpp"

typedef int (*SQLITE_EXEC_CALLBACK)(void* user_data, int columns, char** column_values, char** column_names);

class Sqlite3Helper
{
   private:
    //
    PthreadMutex m_mtx;
    //
    sqlite3* m_handle = NULL;
    //
    std::string m_fn;
    //
    std::string m_sql;

    //
    int _init()
    {
        int ret = -1;
        //
        if (m_handle)
        {
            return 0;
        }
        //
        ret = sqlite3_open(m_fn.c_str(), &m_handle);
        if (ret != SQLITE_OK)
        {
            OS_LOGE("failed to open db");
            return ret;
        }
        OS_LOGI("sqlite3_open(%p)", m_handle);

        return ret;
    }

    int _deinit()
    {
        if (m_handle)
        {
            OS_LOGI("sqlite3_close(%p)", m_handle);
            sqlite3_close(m_handle);
            m_handle = NULL;
        }
        return 0;
    }

    int _reinit()
    {
        _deinit();
        return _init();
    }

    bool _ready()
    {
        return m_handle;
    }

    int _exec_sql_impl(const std::string& sql, SQLITE_EXEC_CALLBACK callback, void* user_data)
    {
        int ret = -1;
        //
        _init();
        //
        if (!_ready())
        {
            OS_LOGV("unready!");
            return ret;
        }
        m_sql = sql;
        //
        char* error_message = NULL;
        //
        ret = sqlite3_exec(m_handle, m_sql.c_str(), callback, user_data, &error_message);
        if (ret != SQLITE_OK)
        {
            OS_LOGE("failed to exec: %s\nerr:%s", m_sql.c_str(), error_message);
            sqlite3_free(error_message);
        }
        return ret;
    }

   public:
    Sqlite3Helper(const std::string& database_file = std::string())
    {
        setDatabaseFile(database_file);
    }
    ~Sqlite3Helper()
    {
        deinit();
    }

    void setDatabaseFile(const std::string& database_file)
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        m_fn = database_file;
    }

    int init()
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _init();
    }

    int deinit()
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _deinit();
    }

    int reinit()
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _reinit();
    }

    bool ready()
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _ready();
    }

    int exec_sql_impl(const std::string& sql, SQLITE_EXEC_CALLBACK callback, void* user_data)
    {
        PthreadMutex::Writelock lock(m_mtx);
        //
        return _exec_sql_impl(sql, callback, user_data);
    }

    static int exec(const std::string& db, const std::string& sql, SQLITE_EXEC_CALLBACK callback, void* user_data)
    {
        Sqlite3Helper obj(db);
        //
        return obj.exec_sql_impl(sql, callback, user_data);
    }
};


#endif  // __CPPHELPER_SQLITE3_H__
