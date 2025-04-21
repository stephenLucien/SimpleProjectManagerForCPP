#ifndef __CMDLINE_ARGUMENT_MANAGER_H__
#define __CMDLINE_ARGUMENT_MANAGER_H__

#include "utils/arg_parser.h"

#ifdef __cplusplus
extern "C" {
#endif


int reg_cmdline_arg_parser(int val, const char* name, int has_arg, ArgumentAtExist atArgExist);

int unreg_cmdline_arg_parser(int val);

int run_cmdline_arg_parser(int argc, char* argv[]);


#define REG_CMDLINE_ARG_PARSE_FUNC(tag, val, name, has_arg, parseArgCode)         \
    static void arg_parser_##tag(int _val, const char* _name, const char* _param) \
    {                                                                             \
        parseArgCode                                                              \
    }                                                                             \
    __attribute__((constructor)) static void reg_cmdline_arg_parser_##tag()       \
    {                                                                             \
        reg_cmdline_arg_parser(val, name, has_arg, arg_parser_##tag);             \
    }


#ifdef __cplusplus
}
#endif


#endif  // __CMDLINE_ARGUMENT_MANAGER_H__
