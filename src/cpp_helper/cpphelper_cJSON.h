#ifndef __CPPHELPER_CJSON_H__
#define __CPPHELPER_CJSON_H__

#include <cjson/cJSON.h>


#define CPP_CJSON_PARSE(name, str)                                                     \
    std::unique_ptr<cJSON, void (*)(cJSON*)> name##_cpp_ptr(cJSON_Parse(str),          \
                                                            [](cJSON* ptr)             \
                                                            {                          \
                                                                if (ptr)               \
                                                                {                      \
                                                                    cJSON_Delete(ptr); \
                                                                }                      \
                                                            });                        \
    cJSON*                                   name = name##_cpp_ptr.get()


#define CPP_CJSON_PRINT(name, js)                                                  \
    std::unique_ptr<char, void (*)(char*)> name##_cpp_ptr(cJSON_Print(js),         \
                                                          [](char* ptr)            \
                                                          {                        \
                                                              if (ptr)             \
                                                              {                    \
                                                                  cJSON_free(ptr); \
                                                              }                    \
                                                          });                      \
    char*                                  name = name##_cpp_ptr.get()

#define CPP_CJSON_PRINT_UNFORMAT(name, js)                                            \
    std::unique_ptr<char, void (*)(char*)> name##_cpp_ptr(cJSON_PrintUnformatted(js), \
                                                          [](char* ptr)               \
                                                          {                           \
                                                              if (ptr)                \
                                                              {                       \
                                                                  cJSON_free(ptr);    \
                                                              }                       \
                                                          });                         \
    char*                                  name = name##_cpp_ptr.get()



#define EASY_CJSON_OBJ(obj, key)                     \
    cJSON* js_##key = NULL;                          \
    do                                               \
    {                                                \
        if (!(obj))                                  \
            break;                                   \
        if ((obj)->type != cJSON_Object)             \
            break;                                   \
        js_##key = cJSON_GetObjectItem((obj), #key); \
    } while (0)

#define EASY_CJSON_OBJ_VAL_STR(obj, key, def)        \
    const char* key      = def;                      \
    cJSON*      js_##key = NULL;                     \
    do                                               \
    {                                                \
        if (!(obj))                                  \
            break;                                   \
        if ((obj)->type != cJSON_Object)             \
            break;                                   \
        js_##key = cJSON_GetObjectItem((obj), #key); \
        if (!js_##key)                               \
            break;                                   \
        if (js_##key->type == cJSON_String)          \
            key = js_##key->valuestring;             \
    } while (0)

#define EASY_CJSON_OBJ_VAL_BOOL(obj, key, def)                             \
    bool   key      = def;                                                 \
    cJSON* js_##key = NULL;                                                \
    do                                                                     \
    {                                                                      \
        if (!(obj))                                                        \
            break;                                                         \
        if ((obj)->type != cJSON_Object)                                   \
            break;                                                         \
        js_##key = cJSON_GetObjectItem((obj), #key);                       \
        if (!js_##key)                                                     \
            break;                                                         \
        if (js_##key->type == cJSON_False || js_##key->type == cJSON_True) \
            key = js_##key->valueint;                                      \
    } while (0)

#define EASY_CJSON_OBJ_VAL_DOUBLE(obj, key, def)     \
    double key      = def;                           \
    cJSON* js_##key = NULL;                          \
    do                                               \
    {                                                \
        if (!(obj))                                  \
            break;                                   \
        if ((obj)->type != cJSON_Object)             \
            break;                                   \
        js_##key = cJSON_GetObjectItem((obj), #key); \
        if (!js_##key)                               \
            break;                                   \
        if (js_##key->type == cJSON_Number)          \
            key = js_##key->valuedouble;             \
    } while (0)


#endif  // __CPPHELPER_CJSON_H__
