#ifndef __CPPHELPER_NLOHMANN_JSON_H__
#define __CPPHELPER_NLOHMANN_JSON_H__

#include "nlohmann/json.hpp"

//
#include "utils/os_tools_log.h"

static inline nlohmann::json nlohmann_js_parse(const std::string& js_string, const nlohmann::json& js_def = nlohmann::json::object())
{
    nlohmann::json js = js_def;
    try
    {
        js = nlohmann::json::parse(js_string);
    } catch (nlohmann::json::parse_error& e)
    {
        OS_LOGE("nlohmann::json err: %s", e.what());
    }
    return js;
}


/**
 * @brief get string or integer/double value from nlohmann json object
 *
 */
#define nlohmann_jsobj_try_get_value_impl(js, key, var, b_chk_type, b_log)                                                                           \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if ((js).empty() || !(js).is_object())                                                                                                       \
            break;                                                                                                                                   \
        if ((js).find(key) == (js).end())                                                                                                            \
        {                                                                                                                                            \
            if (b_log)                                                                                                                               \
            {                                                                                                                                        \
                OS_LOGV("nlohmann jsobj, key not found: %s", std::string(key).c_str());                                                              \
            }                                                                                                                                        \
            break;                                                                                                                                   \
        }                                                                                                                                            \
        if (b_chk_type)                                                                                                                              \
        {                                                                                                                                            \
            bool b_interrupt = false;                                                                                                                \
            try                                                                                                                                      \
            {                                                                                                                                        \
                if (typeid(var) == typeid(std::string))                                                                                              \
                {                                                                                                                                    \
                    if (!(js)[key].is_string())                                                                                                      \
                    {                                                                                                                                \
                        if (b_log)                                                                                                                   \
                        {                                                                                                                            \
                            OS_LOGE(                                                                                                                 \
                                "json type not match, expect string, var type:"                                                                      \
                                "%s, but key '%s' provided %s",                                                                                      \
                                typeid(var).name(),                                                                                                  \
                                std::string(key).c_str(),                                                                                            \
                                (js)[key].type_name());                                                                                              \
                        }                                                                                                                            \
                        b_interrupt = true;                                                                                                          \
                        break;                                                                                                                       \
                    }                                                                                                                                \
                }                                                                                                                                    \
                if (typeid(var) == typeid(int8_t) || typeid(var) == typeid(uint8_t) || typeid(var) == typeid(char) || typeid(var) == typeid(int16_t) \
                    || typeid(var) == typeid(uint16_t) || typeid(var) == typeid(short) || typeid(var) == typeid(int32_t)                             \
                    || typeid(var) == typeid(uint32_t) || typeid(var) == typeid(int) || typeid(var) == typeid(long)                                  \
                    || typeid(var) == typeid(int64_t) || typeid(var) == typeid(uint64_t) || typeid(var) == typeid(long long)                         \
                    || typeid(var) == typeid(float) || typeid(var) == typeid(double) || typeid(var) == typeid(bool))                                 \
                {                                                                                                                                    \
                    if (!(js)[key].is_number() && !(js)[key].is_boolean())                                                                           \
                    {                                                                                                                                \
                        if (b_log)                                                                                                                   \
                        {                                                                                                                            \
                            OS_LOGE(                                                                                                                 \
                                "json type not match, expect boolean or number, var type:"                                                           \
                                "%s, but key '%s' provided %s",                                                                                      \
                                typeid(var).name(),                                                                                                  \
                                std::string(key).c_str(),                                                                                            \
                                (js)[key].type_name());                                                                                              \
                        }                                                                                                                            \
                        b_interrupt = true;                                                                                                          \
                        break;                                                                                                                       \
                    }                                                                                                                                \
                }                                                                                                                                    \
            } catch (std::exception & e)                                                                                                             \
            {                                                                                                                                        \
                OS_LOGE("nlohmann err: %s", e.what());                                                                                               \
            }                                                                                                                                        \
            if (b_interrupt)                                                                                                                         \
            {                                                                                                                                        \
                break;                                                                                                                               \
            }                                                                                                                                        \
        }                                                                                                                                            \
        try                                                                                                                                          \
        {                                                                                                                                            \
            (var) = (js)[key];                                                                                                                       \
        } catch (nlohmann::json::exception & e)                                                                                                      \
        {                                                                                                                                            \
            OS_LOGE("nlohmann::json err: %s", e.what());                                                                                             \
        }                                                                                                                                            \
    } while (0)

#define nlohmann_jsobj_try_get_value_debug(js, key, var) nlohmann_jsobj_try_get_value_impl(js, key, var, 1, 1)
#define nlohmann_jsobj_try_get_value(js, key, var)       nlohmann_jsobj_try_get_value_impl(js, key, var, 1, 0)
#define nlohmann_jsobj_try_get_keyvalue(js, keyvar)      nlohmann_jsobj_try_get_value_impl(js, #keyvar, keyvar, 1, 0)

#define nlohmann_jsobj_get_value_debug(js, key, var) nlohmann_jsobj_try_get_value_impl(js, key, var, 0, 1)
#define nlohmann_jsobj_get_value(js, key, var)       nlohmann_jsobj_try_get_value_impl(js, key, var, 0, 0)
#define nlohmann_jsobj_get_keyvalue(js, keyvar)      nlohmann_jsobj_try_get_value_impl(js, #keyvar, keyvar, 0, 0)

#define nlohmann_js_extract(js, key, x)              \
    try                                              \
    {                                                \
        (x) = (js)[key].get<decltype(x)>();          \
    } catch (nlohmann::json::exception & e)          \
    {                                                \
        OS_LOGE("nlohmann::json err: %s", e.what()); \
    }



#endif  // __CPPHELPER_NLOHMANN_JSON_H__
