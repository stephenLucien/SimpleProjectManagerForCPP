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
#include <list>
#include <string>
#include <vector>
//
namespace fs = std::filesystem;

std::list<std::string> glob_json_file(const std::string& dst_path)
{
    auto tmpPath = fs::current_path();
    //
    fs::current_path(dst_path);
    //
    auto current_dir = fs::current_path();
    //
    std::list<std::string> jsFiles;
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
            jsFiles.push_back(file.filename());
        } else
        {
            char buf[4096];
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "%s/%s", file.parent_path().make_preferred().c_str(), file.filename().c_str());
            jsFiles.push_back(buf);
        }
    }
    //
    fs::current_path(tmpPath);
    return jsFiles;
}
