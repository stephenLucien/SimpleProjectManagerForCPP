#include <inttypes.h>
#include <unistd.h>
#include <algorithm>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
//
#include "json2cppclass/Json2Class.hpp"
#include "json2cppclass/globJsonFile.h"
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

namespace fs = std::filesystem;

static std::string m_top_dir = "Json2ClassTest";

static int test_pathclass(int reason, void* userdata)
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

REG_TEST_FUNC(test_pathclass, test_pathclass, NULL)


static int test_jsonclass_glob(int reason, void* userdata)
{
    auto data = JsonToClassFactory::globJsonClass(m_top_dir);
    //
    auto gen2dir = "src/json2cppclass/test";
    //
    std::unordered_map<std::string, std::string> js2hppMap;
    std::unordered_map<std::string, std::string> c2hppMap;
    for (auto& [f, p] : data)
    {
        //
        auto hpp = f;
        //
        hpp += ".hpp";
        std::replace_if(hpp.begin(), hpp.end(), [](char c) { return c == '/'; }, '_');
        //
        js2hppMap[f] = hpp;
        //
        std::string c, ns, cn;
        //
        c = JsonToClassFactory::path2class(f, ns, cn);
        //
        c2hppMap[c] = hpp;
    }
    //
    auto gheader = R"(
        #pragma once
        #include <cstddef>
        #include <cstdint>
        #include <nlohmann/json.hpp>
        //
        #include "json2cppclass/JsonClassAPI.hpp"
        //
        )";
    //
    for (auto& [f, p] : data)
    {
        //
        auto hpp = js2hppMap[f];
        //
        std::list<std::string> refClass;
        //
        auto content = JsonToClassFactory::Json2Class(p, refClass);
        //
        std::string output = gheader;
        //
        for (auto& rc : refClass)
        {
            auto tmphpp = c2hppMap.find(rc);
            if (tmphpp == c2hppMap.end())
            {
                continue;
            }
            //
            output += "#include \"";
            output += tmphpp->second;
            output += "\"\n";
        }
        //
        output += "\n";
        output += content;
        //
        {
            //
            SYSTEM("mkdir -p %s", gen2dir);
            //
            char buf[4096];
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s/%s", gen2dir, hpp.c_str());
            //
            auto hppf = buf;
            write_text_file(hppf, output);
            //
            SYSTEM("clang-format -i %s", hppf);
        }
    }


    return 0;
}

REG_TEST_FUNC(test_jsonclass_glob, test_jsonclass_glob, NULL)
