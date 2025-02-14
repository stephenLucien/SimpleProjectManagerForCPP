#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ArgumentAtExist)(int val, const char* name, const char* param);

typedef struct
{
    //
    int val;
    //
    char name[64];
    //
    int has_arg;
    //
    ArgumentAtExist atArgExist;
} ArgumentData;

int arg_parser(const ArgumentData* arg_datas, int arg_datas_cnt, int argc, char* argv[]);


#ifdef __cplusplus
}
#endif

#endif  // __ARG_PARSER_H__
