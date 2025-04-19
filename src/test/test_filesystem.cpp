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
#include "json2cppclass/Json2Class.hpp"
#include "json2cppclass/globJsonFile.h"
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

namespace fs = std::filesystem;

static std::string m_top_dir = "Json2ClassTest";

static int test_dir_itr(int reason, void* userdata)
{
    int ret = -1;
    //
    auto jsFiles = glob_json_file(m_top_dir);
    //
    for (auto& e : jsFiles)
    {
        std::string ns, cn;
        //
        auto c = JsonToClassFactory::path2class(e, ns, cn);
        //
        OS_LOGD("path: %s, ns:(%s), cn:(%s), c:(%s)", e.c_str(), ns.c_str(), cn.c_str(), c.c_str());
        //
        std::string rns, rcn;
        JsonToClassFactory::splitClass2Name(c, rns, rcn);
        //
        OS_LOGD("path: %s, ns:(%s), cn:(%s), c:(%s)", e.c_str(), rns.c_str(), rcn.c_str(), c.c_str());
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


static int test_jsonclass_glob(int reason, void* userdata)
{
    auto output = JsonToClassFactory::genHpp(m_top_dir);
    //
    auto hppf = "src/json2cppclass/AutoJs.hpp";
    write_text_file(hppf, output);

    SYSTEM("clang-format -i %s", hppf);

    return 0;
}

REG_TEST_FUNC(test_jsonclass_glob, test_jsonclass_glob, NULL)
