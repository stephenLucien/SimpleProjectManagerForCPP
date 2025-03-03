#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <string>
#include <vector>
//
#include "cpp_helper/cpphelper_sqlite3.hpp"
#include "manager/test_manager.h"
#include "utils/os_tools.h"
#include "utils/os_tools_log.h"

//
#define CALL_LOG_DATABASE "call_log.db"
//
#define TABLE_NAME "log1"


/**
 * 通话记录
 */
class CallLogEntry
{
   public:
    int id;
    /**
     * ip
     */
    std::string uri;
    /**
     * 联系人
     */
    std::string contact;
    // 联系人别名
    std::string contact_alias;
    // 通话别名
    std::string display_name;

    /**
     * @brief 通话方向
     * \ref enum CallDirection
     */
    int direction;
    /**
     * 通话时长
     */
    unsigned int duration;
    /**
     * 创建时间
     */
    unsigned int created_at;

    void dump()
    {
        OS_PRINT("id=%d, uri=%s, contact=%s, contact_alias=%s, display_name=%s, direction=%d, duration=%u, ts=%u",
                 id,
                 uri.c_str(),
                 contact.c_str(),
                 contact_alias.c_str(),
                 display_name.c_str(),
                 direction,
                 duration,
                 created_at);
    }
};

typedef std::vector<CallLogEntry> CallLogEntries;


static int QueryCountCallback(void* user_data, int columns, char** column_values, char** column_names)
{
    //
    if (columns != 8)
    {
        OS_LOGE("columns not match");
        return 0;
    }
    auto pn = (int*)user_data;
    //
    ++pn[0];

    return 0;
}

int GetCallLogCount()
{
    int n = 0;
    Sqlite3Helper::exec(CALL_LOG_DATABASE, "SELECT * from " TABLE_NAME "", QueryCountCallback, &n);
    return n;
}

int PutCallLog(const CallLogEntry& log)
{
    Sqlite3Helper obj(CALL_LOG_DATABASE);

    // 创建表，确保表存在
    std::string sql = "CREATE TABLE if not exists " TABLE_NAME
                      "("
                      "id integer PRIMARY KEY AUTOINCREMENT NOT NULL, "
                      "uri char(64) NOT NULL, "
                      "contact char(64) NOT NULL, "
                      "contact_alias char(64) NOT NULL, "
                      "display_name char(64) NOT NULL, "
                      "direction integer NOT NULL, "
                      "duration unsigned int NOT NULL, "
                      "created_at  unsigned int NOT NULL);";
    if (0 != obj.exec_sql_impl(sql, NULL, NULL))
    {
        return -1;
    }

    if (0 != obj.exec_sql_impl("BEGIN TRANSACTION;", NULL, NULL))
    {
        return -1;
    }

    char str[1024];
    snprintf(str,
             sizeof(str),
             "INSERT INTO " TABLE_NAME
             " ('uri', 'contact', 'contact_alias', 'display_name', 'direction', 'duration', 'created_at')"
             " values ('%s', '%s', '%s', '%s', %d, %u, %u);",
             log.uri.c_str(),
             log.contact.c_str(),
             log.contact_alias.c_str(),
             log.display_name.c_str(),
             log.direction,
             log.duration,
             time(NULL));

    if (0 != obj.exec_sql_impl(str, NULL, NULL))
    {
        obj.exec_sql_impl("ROLLBACK;", NULL, NULL);
        return -1;
    }

    if (0 != obj.exec_sql_impl("END TRANSACTION;", NULL, NULL))
    {
        return -1;
    }
    return 0;
}

static int QueryRowCallback(void* user_data, int columns, char** column_values, char** column_names)
{
    if (columns != 8)
    {
        OS_LOGE("columns not match");
        return 0;
    }
    CallLogEntries* entries = (CallLogEntries*)user_data;
    try
    {
        CallLogEntry entry;
        //
        entry.id            = std::atoi(column_values[0]);
        entry.uri           = column_values[1];
        entry.contact       = column_values[2];
        entry.contact_alias = column_values[3];
        entry.display_name  = column_values[4];
        entry.direction     = std::atoi(column_values[5]);
        entry.duration      = std::atoi(column_values[6]);
        entry.created_at    = std::atoi(column_values[7]);
        //
        entries->push_back(entry);
    } catch (std::exception& e)
    {
        OS_LOGE("<%s,%d> err: %s", __FUNCTION__, __LINE__, e.what());
    }

    return 0;
}

int GetCallLog(CallLogEntries* records)
{
    if (!records)
    {
        return -1;
    }
    records->clear();
    //
    return Sqlite3Helper::exec(CALL_LOG_DATABASE, "SELECT * from " TABLE_NAME "", QueryRowCallback, records);
}

int DeleteOldCallLog(int n)
{
    char str[1024];
    snprintf(str, sizeof(str), "delete from " TABLE_NAME " where id in(select id from " TABLE_NAME " order by id limit %d)", n);
    //
    return Sqlite3Helper::exec(CALL_LOG_DATABASE, str, NULL, NULL);
}

static int sqlite3_test(int reason, void* userdata)
{
    OS_LOGD("before!!!");
    //
    auto n = GetCallLogCount();
    OS_LOGD("n=%d", n);
    //
    CallLogEntries records;
    GetCallLog(&records);
    for (auto& e : records)
    {
        e.dump();
    }
    //
    OS_LOGD("edit!!!");
    CallLogEntry entry;
    //
    entry.uri = "123";
    entry.contact = "li";
    entry.direction = 2;
    entry.duration = 33;
    PutCallLog(entry);


    OS_LOGD("after!!!");
    GetCallLog(&records);
    for (auto& e : records)
    {
        e.dump();
    }

    return 0;
}

REG_TEST_FUNC(sqlite3_test, sqlite3_test, NULL)
