#include <inttypes.h>
#include <unistd.h>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>
//
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"

namespace fs = std::filesystem;

static std::string m_top_dir = "Json2ClassTest";

static int test_dir_itr(int reason, void* userdata)
{
    int ret = -1;
    // cd m_top_dir
    fs::current_path(m_top_dir);
    //
    auto current_dir = fs::current_path();
    //
    std::cout << "Current path is " << current_dir << '\n';
    //
    for (auto const& dir_entry : fs::recursive_directory_iterator(current_dir))
    {
        if (dir_entry.is_directory())
        {
            continue;
        }
        auto file = fs::relative(dir_entry.path(), current_dir);
        OS_LOGD("dirname: %s", file.parent_path().c_str());
        OS_LOGD("fn: %s", file.filename().c_str());
        OS_LOGD("ext: %s", file.extension().c_str());
    }


    return 0;
}

REG_TEST_FUNC(test_dir_itr, test_dir_itr, NULL)


static int test_glob_json_file(int reason, void* userdata)
{
    fs::current_path(m_top_dir);
    //
    auto current_dir = fs::current_path();
    //
    for (auto const& dir_entry : fs::recursive_directory_iterator(current_dir))
    {
        if (dir_entry.is_directory())
        {
            continue;
        }
        auto file = fs::relative(dir_entry.path(), current_dir);
        if (strcasecmp(file.extension().c_str(), ".json") != 0)
        {
            continue;
        }
        if (file.parent_path().empty())
        {
            OS_LOGD("%s", file.filename().c_str());
        } else
        {
            OS_LOGD("%s/%s", file.parent_path().c_str(), file.filename().c_str());
        }
    }


    return 0;
}

REG_TEST_FUNC(test_glob_json_file, test_glob_json_file, NULL)
