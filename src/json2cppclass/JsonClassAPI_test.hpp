#ifndef __JSONCLASSAPI_TEST_H__
#define __JSONCLASSAPI_TEST_H__



#include <nlohmann/json.hpp>
#include "MyJsonParser/JsonClassAPI.hpp"

class TestJsonClassAPI : public nlohmannUser::JsonClassAPI
{
   protected:
    //
    int id;
    //
    std::string name;
    //
    double value;

   protected:
    //
    void _clear() override
    {
        //
        id = -1;
        //
        name.clear();
        //
        value = 0.0;
    }
    //
    int _fromJson(const nlohmann::json& js) override
    {
        //
        id = _JsonGetValueRet(js, "id", id);
        //
        name = _JsonGetStringRet(js, "name", name);
        //
        value = _JsonGetValueRet(js, "value", value);

        return _isValid() ? 0 : -1;
    }
    //
    nlohmann::json _toJson() const override
    {
        auto js = nlohmann::json::object();
        //
        js["id"] = id;
        //
        js["name"] = name;
        //
        js["value"] = value;

        return js;
    }
    //
    bool _isValid() const override
    {
        return id >= 0;
    }
};


#endif  // __JSONCLASSAPI_TEST_H__
