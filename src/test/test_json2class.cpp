#include <inttypes.h>
#include <libgen.h>
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
#include "manager/cmdline_argument_manager.h"
#include "manager/test_manager.h"
#include "utils/os_tools_log.h"
#include "utils/os_tools_system.h"

namespace fs = std::filesystem;

//
static std::string m_src_dir = "Json2ClassTest";
//
REG_CMDLINE_ARG_PARSE_FUNC(url, 'S', "srcDir", 1, {
    //
    if (_param)
    {
        m_src_dir = _param;
    }
})
//
static std::string m_dst_dir;
//
REG_CMDLINE_ARG_PARSE_FUNC(path, 'D', "dstDir", 1, {
    //
    if (_param)
    {
        m_dst_dir = _param;
    }
})


static int test_pathclass(int reason, void* userdata)
{
    int ret = -1;
    //
    auto jsFiles = glob_json_file(m_src_dir);
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


//
static int test_jsonclass_glob(int reason, void* userdata)
{
    auto data = JsonToClassFactory::globJsonClass(m_src_dir);
    //
    auto gen2dir = m_dst_dir;
    if (gen2dir.empty())
    {
        gen2dir = m_src_dir;
    }
    //
    std::unordered_map<std::string, std::string> js2hppMap;
    std::unordered_map<std::string, std::string> c2hppMap;
    for (auto& [f, p] : data)
    {
        //
        auto pext = strcasestr(f.c_str(), ".json");
        //
        auto sf = f;
        if (pext)
        {
            auto ext = std::string(pext);
            //
            sf = f.substr(0, f.length() - ext.length());
        }
        //
        auto hpp = sf;
        //
        std::replace_if(hpp.begin(), hpp.end(), [](char c) { return c == '/'; }, '_');
        //
        hpp += ".hpp";
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
        #include "MyJsonParser/JsonClassAPI.hpp"
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
            SYSTEM("mkdir -p %s", gen2dir.c_str());
            //
            char buf[4096];
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s/%s", gen2dir.c_str(), hpp.c_str());
            //
            auto hppf = buf;
            write_text_file(hppf, output);
            //
            SYSTEM("clang-format -i %s", hppf);
        }
    }
    OS_LOGD("");

    //
    SYSTEM("mkdir -p %s/%s", gen2dir.c_str(), "MyJsonParser");

    if (access("src/MyJsonParser/JsonClassAPI.hpp", F_OK) == 0)
    {
        SYSTEM("cp %s %s/%s/", "src/MyJsonParser/JsonClassAPI.hpp", gen2dir.c_str(), "MyJsonParser");
    } else
    {
        //
        char filepath[4096] = __FILE__;
        //
        auto dn = dirname(filepath);
        //
        if (!dn)
        {
            OS_LOGE("");
        }
        //
        auto apiF = std::string(dn ? dn : ".") + "/../MyJsonParser/JsonClassAPI.hpp";
        //
        OS_LOGD("from %s", apiF.c_str());
        if (access(apiF.c_str(), F_OK) == 0)
        {
            SYSTEM("cp %s %s/%s/", apiF.c_str(), gen2dir.c_str(), "MyJsonParser");
        } else
        {
        }
    }

    return 0;
}
REG_TEST_FUNC(test_jsonclass_glob, test_jsonclass_glob, NULL)
