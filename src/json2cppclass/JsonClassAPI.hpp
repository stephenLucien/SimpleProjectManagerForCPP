#ifndef __JSONCLASSAPI_H__
#define __JSONCLASSAPI_H__


#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

//
#include <nlohmann/json.hpp>
#include "utils/os_tools_system.h"

namespace nlohmannUser
{
#define nlohmannUserArrayContainer(type) std::vector<type>
#define UNUSED_JS_KEY(key)
#define UNUSED_JS_ARR_KEY(key)

class JsonClassAPI
{
   protected:
    //
    virtual void _clear()
    {
    }
    //
    virtual int _fromJson(const nlohmann::json& js)
    {
        return -1;
    }
    //
    virtual nlohmann::json _toJson() const
    {
        return nlohmann::json::object();
    }
    //
    virtual bool _isValid() const
    {
        return false;
    }

   public:
    JsonClassAPI(const nlohmann::json& js = nlohmann::json::object())
    {
        fromJson(js);
    }
    virtual ~JsonClassAPI()
    {
        clear();
    }
    //
    int fromJson(const nlohmann::json& js)
    {
        clear();
        //
        return _fromJson(js);
    }
    //
    nlohmann::json toJson() const
    {
        return _toJson();
    }
    //
    static nlohmann::json parseJsonString(const std::string& jsStr)
    {
        nlohmann::json js;
        try
        {
            js = nlohmann::json::parse(jsStr, nullptr, true, true);
        } catch (nlohmann::json::parse_error& e)
        {
            // OS_LOGE("nlohmann::json err: %s", e.what());
        }
        return js;
    }
    //
    int fromJsonString(const std::string& jsStr)
    {
        return fromJson(parseJsonString(jsStr));
    }
    //
    std::string toJsonString(int indent = -1) const
    {
        return _toJson().dump(indent);
    }
    //
    int fromJsonFile(const std::string& fp, size_t bufsz = 1024 * 128)
    {
        if (!bufsz)
        {
            return -1;
        }
        std::vector<char> buf(bufsz);
        memset(buf.data(), 0, bufsz);
        read_data_from_file(fp.c_str(), buf.data(), bufsz - 1, NULL);
        //
        return fromJsonString(buf.data());
    }
    //
    int toJsonFile(const std::string& fp, int intent = -1)
    {
        return write_text_file(fp, toJsonString(intent));
    }
    //
    bool isValid() const
    {
        return _isValid();
    }
    //
    void clear()
    {
        _clear();
    }
    //
    static bool _JsonToBool(const nlohmann::json& js, bool& value, bool def = false)
    {
        value = def;
        if (!js.is_boolean())
        {
            return false;
        }
        value = js.get<decltype(def)>();
        return true;
    }
    //
    static double _JsonToBoolRet(const nlohmann::json& js, bool def = false)
    {
        auto value = def;
        _JsonToBool(js, value, def);
        return value;
    }
    //
    static bool _JsonGetBool(const nlohmann::json& js, const std::string& key, bool& value, bool def = false)
    {
        value = def;
        //
        if (!js.is_object())
        {
            return false;
        }
        //
        if (key.empty())
        {
            return false;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return false;
        }
        //
        return _JsonToBool(*itr, value, def);
    }
    //
    static bool _JsonGetBoolRet(const nlohmann::json& js, const std::string& key, bool def = false)
    {
        bool value = def;
        _JsonGetBool(js, key, value, def);
        return value;
    }
    //
    template <class T>
    static bool _JsonToValue(const nlohmann::json& js, T& value, T def = 0.0)
    {
        value = def;
        if (!js.is_number())
        {
            return false;
        }
        value = js.get<T>();
        return true;
    }
    //
    template <class T>
    static T _JsonToValueRet(const nlohmann::json& js, T def = 0.0)
    {
        auto value = def;
        _JsonToValue(js, value, def);
        return value;
    }
    //
    template <class T>
    static bool _JsonGetValue(const nlohmann::json& js, const std::string& key, T& value, T def = 0.0)
    {
        value = def;
        //
        if (!js.is_object())
        {
            return false;
        }
        //
        if (key.empty())
        {
            return false;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return false;
        }
        //
        return _JsonToValue(*itr, value, def);
    }
    //
    template <class T>
    static T _JsonGetValueRet(const nlohmann::json& js, const std::string& key, T def = 0.0)
    {
        auto value = def;
        //
        _JsonGetValue(js, key, value, def);
        return value;
    }
    //
    static bool _JsonToString(const nlohmann::json& js, std::string& value, std::string def = std::string())
    {
        value = def;
        if (!js.is_string())
        {
            return false;
        }
        value = js.get<decltype(def)>();
        return true;
    }
    //
    static std::string _JsonToStringRet(const nlohmann::json& js, std::string def = std::string())
    {
        auto value = def;
        _JsonToString(js, value, def);
        return value;
    }
    //
    static bool _JsonGetString(const nlohmann::json& js, const std::string& key, std::string& value, const std::string& def = std::string())
    {
        value = def;
        //
        if (!js.is_object())
        {
            return false;
        }
        //
        if (key.empty())
        {
            return false;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return false;
        }
        //
        return _JsonToString(*itr, value, def);
    }
    //
    static std::string _JsonGetStringRet(const nlohmann::json& js, const std::string& key, const std::string& def = std::string())
    {
        auto value = def;
        _JsonGetString(js, key, value, def);
        return value;
    }
    //
    static bool _JsonToClass(const nlohmann::json& js, JsonClassAPI* pc)
    {
        if (!pc)
        {
            return false;
        }
        pc->fromJson(js);
        return pc->isValid();
    }
    //
    static bool _JsonToClass(const nlohmann::json& js, JsonClassAPI& c)
    {
        return _JsonToClass(js, &c);
    }
    //
    static bool _JsonGetClass(const nlohmann::json& js, const std::string& key, JsonClassAPI& value)
    {
        value.clear();
        //
        if (!js.is_object())
        {
            return false;
        }
        //
        if (key.empty())
        {
            return false;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return false;
        }
        //
        return _JsonToClass(*itr, value);
    }
    //
    static size_t _JsonToArrayBool(const nlohmann::json& js, nlohmannUserArrayContainer(bool) & array, bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_array())
        {
            return cnt;
        }
        //
        array.reserve(js.size());
        //
        for (const auto& e : js)
        {
            bool value;
            //
            auto ret = _JsonToBool(e, value);
            //
            if (ret || !dropInvalid)
            {
                array.push_back(value);
                ++cnt;
            }
        }

        return cnt;
    }
    //
    static size_t _JsonGetArrayBool(const nlohmann::json& js,
                                    const std::string&    key,
                                    nlohmannUserArrayContainer(bool) & array,
                                    bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_object())
        {
            return cnt;
        }
        //
        if (key.empty())
        {
            return cnt;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return cnt;
        }
        //
        cnt = _JsonToArrayBool(*itr, array, dropInvalid);
        //
        return cnt;
    }
    //
    static size_t _JsonToArrayInt(const nlohmann::json& js, nlohmannUserArrayContainer(int64_t) & array, bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_array())
        {
            return cnt;
        }
        //
        array.reserve(js.size());
        //
        for (const auto& e : js)
        {
            double value;
            //
            auto ret = _JsonToValue(e, value);
            //
            if (ret || !dropInvalid)
            {
                array.push_back(value);
                ++cnt;
            }
        }

        return cnt;
    }
    //
    static size_t _JsonGetArrayInt(const nlohmann::json& js,
                                   const std::string&    key,
                                   nlohmannUserArrayContainer(int64_t) & array,
                                   bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_object())
        {
            return cnt;
        }
        //
        if (key.empty())
        {
            return cnt;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return cnt;
        }
        //
        cnt = _JsonToArrayInt(*itr, array, dropInvalid);
        //
        return cnt;
    }
    //
    static size_t _JsonToArrayFloat(const nlohmann::json& js, nlohmannUserArrayContainer(double) & array, bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_array())
        {
            return cnt;
        }
        //
        array.reserve(js.size());
        //
        for (const auto& e : js)
        {
            double value;
            //
            auto ret = _JsonToValue(e, value);
            //
            if (ret || !dropInvalid)
            {
                array.push_back(value);
                ++cnt;
            }
        }

        return cnt;
    }
    //
    static size_t _JsonGetArrayFloat(const nlohmann::json& js,
                                     const std::string&    key,
                                     nlohmannUserArrayContainer(double) & array,
                                     bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_object())
        {
            return cnt;
        }
        //
        if (key.empty())
        {
            return cnt;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return cnt;
        }
        //
        cnt = _JsonToArrayFloat(*itr, array, dropInvalid);
        //
        return cnt;
    }
    //
    static size_t _JsonToArrayString(const nlohmann::json& js, nlohmannUserArrayContainer(std::string) & array, bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_array())
        {
            return cnt;
        }
        //
        array.reserve(js.size());
        //
        for (const auto& e : js)
        {
            std::string value;
            //
            auto ret = _JsonToString(e, value);
            //
            if (ret || !dropInvalid)
            {
                array.push_back(value);
                ++cnt;
            }
        }

        return cnt;
    }
    //
    static size_t _JsonGetArrayString(const nlohmann::json& js,
                                      const std::string&    key,
                                      nlohmannUserArrayContainer(std::string) & array,
                                      bool dropInvalid = true)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_object())
        {
            return cnt;
        }
        //
        if (key.empty())
        {
            return cnt;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return cnt;
        }
        //
        cnt = _JsonToArrayString(*itr, array, dropInvalid);
        //
        return cnt;
    }
    //
    static size_t _JsonToArrayClass(const nlohmann::json& js, nlohmannUserArrayContainer(JsonClassAPI*) & array)
    {
        size_t cnt = 0;
        //
        for (auto& c : array)
        {
            if (!c)
            {
                continue;
            }
            c->clear();
        }
        //
        if (!js.is_array())
        {
            return cnt;
        }
        //
        auto citr = array.begin();
        auto jitr = js.begin();
        for (; citr != array.end() && jitr != js.end(); ++citr, ++jitr)
        {
            //
            auto ret = _JsonToClass(*jitr, *citr);
            //
            if (ret)
            {
                ++cnt;
            }
        }

        return cnt;
    }
    //
    template <class T>
    static size_t _JsonGetArrayClass(const nlohmann::json& js, const std::string& key, nlohmannUserArrayContainer(T) & array)
    {
        size_t cnt = 0;
        //
        array.clear();
        //
        if (!js.is_object())
        {
            return cnt;
        }
        //
        if (key.empty())
        {
            return cnt;
        }
        //
        auto itr = js.find(key);
        if (itr == js.end())
        {
            return cnt;
        }
        array.resize(itr->size());
        //
        nlohmannUserArrayContainer(JsonClassAPI*) parray;
        parray.reserve(array.size());
        for (auto& e : array)
        {
            parray.push_back(&e);
        }
        //
        cnt = _JsonToArrayClass(*itr, parray);
        //
        return cnt;
    }
};

}  // namespace nlohmannUser



#endif  // __JSONCLASSAPI_H__
