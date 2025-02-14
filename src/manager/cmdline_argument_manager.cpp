#include "cmdline_argument_manager.h"


//
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils/arg_parser.h"
#include "utils/os_tools.h"
#include "utils/pthread_class.hpp"

namespace
{


class CmdlineArgParser
{
   private:
    CmdlineArgParser()
    {
    }
    ~CmdlineArgParser()
    {
    }
    PthreadMutex m_mtx;
    //
    std::unordered_map<int, ArgumentData> m_parsers;

   public:
    static CmdlineArgParser* getInstance()
    {
        static CmdlineArgParser obj;
        return &obj;
    }

    int reg(const ArgumentData& data)
    {
        PthreadMutex::Writelock _l(m_mtx);
        //
        m_parsers[data.val] = data;
        return 0;
    }

    int unreg(int val)
    {
        PthreadMutex::Writelock _l(m_mtx);
        //
        auto itr = m_parsers.find(val);
        if (itr == m_parsers.end())
        {
            return 0;
        }
        m_parsers.erase(itr);
        return 1;
    }

    int run(int argc, char* argv[])
    {
        int ret = -1;
        //
        PthreadMutex::Writelock _l(m_mtx);
        //
        std::vector<ArgumentData> data;
        //
        data.reserve(m_parsers.size());
        //
        for (auto& e : m_parsers)
        {
            data.push_back(e.second);
        }
        ret = arg_parser(data.data(), data.size(), argc, argv);
        //
        return ret;
    }
};

}  // namespace


int reg_cmdline_arg_parser(int val, const char* name, int has_arg, ArgumentAtExist atArgExist)
{
    ArgumentData data;
    //
    data.val = val;
    //
    snprintf(data.name, sizeof(data.name), "%s", name);
    //
    data.has_arg = has_arg;
    //
    data.atArgExist = atArgExist;
    //
    return CmdlineArgParser::getInstance()->reg(data);
}

int unreg_cmdline_arg_parser(int val)
{
    return CmdlineArgParser::getInstance()->unreg(val);
}

int run_cmdline_arg_parser(int argc, char* argv[])
{
    return CmdlineArgParser::getInstance()->run(argc, argv);
}
