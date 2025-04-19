#ifndef __JSON2CLASS_H__
#define __JSON2CLASS_H__

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "JsonClassAPI.hpp"
#include "globJsonFile.h"
#include "utils/cstring_proc.h"
#include "utils/os_tools_log.h"



class JsonToClassFactory
{
   public:
    static std::string path2class(const std::string& path, std::string& ns, std::string& cn)
    {
        auto slash_pos = strrchr(path.c_str(), '/');
        //
        auto fn = path;
        auto pd = std::string();
        if (slash_pos)
        {
            fn = slash_pos + 1;
            pd = path.substr(0, path.length() - fn.length() - 1);
        }
        auto pext = strcasestr(fn.c_str(), ".json");
        if (pext)
        {
            auto ext = std::string(pext);
            //
            cn = fn.substr(0, fn.length() - ext.length());
        } else
        {
            cn = fn;
        }
        std::vector<char> nsbuf;
        //
        auto slash_cnt = std::count_if(pd.begin(), pd.end(), [](char c) { return c == '/'; });
        nsbuf.resize(pd.length() + 1 + slash_cnt);
        memset(nsbuf.data(), 0, nsbuf.size());
        for (size_t si = 0, di = 0; si < pd.length() && di < nsbuf.size(); ++si)
        {
            if (pd[si] == '/')
            {
                nsbuf[di++] = ':';
                nsbuf[di++] = ':';
                continue;
            }
            nsbuf[di++] = pd[si];
        }
        ns = nsbuf.data();

        return ns.empty() ? cn : (ns + "::" + cn);
    }

    static void splitClass2Name(const std::string& c, std::string& ns, std::string& cn)
    {
        ns.clear();
        cn.clear();
        std::vector<char> cbuf(c.length() + 1);
        memcpy(cbuf.data(), c.c_str(), c.length() + 1);
        std::vector<char*> tokens;
        str2tokens(cbuf.data(), tokens, "::");
        if (tokens.empty())
        {
            cn = c;
            return;
        }
        cn = tokens.back();
        //
        for (size_t i = 0; i < tokens.size() - 1; ++i)
        {
            if (ns.empty())
            {
                ns = tokens[i];
                continue;
            }
            ns += "::";
            ns += tokens[i];
        }
    }

    /**
     * @brief
     *
     * @param content
     * @return std::string
     * - "\\Ref:private/A.json" : private/A.json
     *
     */
    static std::string getRefPathFromString(const std::string& content)
    {
        if (strncasecmp(content.c_str(), "\\Ref:", strlen("\\Ref:")) != 0)
        {
            return std::string();
        }
        return content.substr(strlen("\\Ref:"));
    }

    static std::string getRefClassFromString(const std::string& content, std::string& ns, std::string& cn)
    {
        auto path = getRefPathFromString(content);
        //
        ns.clear();
        cn.clear();
        if (path.empty())
        {
            return std::string();
        }
        //
        auto c = path2class(path, ns, cn);
        //
        splitClass2Name(c, ns, cn);
        //
        return ns.empty() ? cn : (ns + "::" + cn);
    }

    class JsonClassApiHelper : public nlohmannUser::JsonClassAPI
    {
       protected:
        nlohmann::json js;

        //
        void _clear() override
        {
            js = nlohmann::json();
        }
        //
        int _fromJson(const nlohmann::json& js) override
        {
            this->js = js;
            return 0;
        }
        //
        nlohmann::json _toJson() const override
        {
            return js;
        }
        //
        bool _isValid() const override
        {
            // return !js.is_null();
            return true;
        }

       public:
        static std::string Json2ClassOneLevel(const std::pair<std::string, JsonClassApiHelper>&      input,
                                              std::list<std::pair<std::string, JsonClassApiHelper>>& leftClass,
                                              std::list<std::string>&                                refClass,
                                              std::string&                                           parent_ns)
        {
            std::string output;
            //
            const auto& js = input.second.js;
            //
            const auto& input_name = input.first;
            //
            std::string input_ns, input_cn;
            //
            splitClass2Name(input_name, input_ns, input_cn);
            //
            if (parent_ns.empty())
            {
                parent_ns = input_ns;
            }
            //
            output += "namespace ";
            output += parent_ns;
            output += " {\n";
            //
            output += "class ";
            output += input_cn;
            output += ": public nlohmannUser::JsonClassAPI\n";
            output += "{\n";
            //
            output += " public:\n";
            //
            std::string eleString, clearString, fromString, toString;
            //
            eleString = R"(
//
bool mIsValid;
            )";
            //
            clearString = R"(
//
mIsValid = false;
               )";
            //
            auto hdl_keyval = [&](const std::string& key, const nlohmann::json& value)
            {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                if (value.is_null())
                {
                    snprintf(buf, sizeof(buf), "//\nUNUSED_JS_KEY(%s);\n", key.c_str());
                    eleString += buf;
                    //
                    snprintf(buf, sizeof(buf), "// clear (%s) \n", key.c_str());
                    clearString += buf;
                    //
                    snprintf(buf, sizeof(buf), "// extract (%s) \n", key.c_str());
                    fromString += buf;
                    //
                    snprintf(buf, sizeof(buf), "// tojs (%s) \n", key.c_str());
                    toString += buf;

                    // OS_LOGV("%s", output.c_str());
                    return;
                }
                if (value.is_boolean())
                {
                    snprintf(buf, sizeof(buf), "//\nbool %s;\n", key.c_str());
                    eleString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\n%s = false;\n", key.c_str());
                    clearString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetBool(js, \"%s\", %s, %s);\n", key.c_str(), key.c_str(), key.c_str());
                    fromString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\njs[\"%s\"] = %s;\n", key.c_str(), key.c_str());
                    toString += buf;

                    // OS_LOGV("%s", output.c_str());
                    return;
                }
                if (value.is_number_float())
                {
                    snprintf(buf, sizeof(buf), "//\ndouble %s;\n", key.c_str());
                    eleString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\n%s = 0.0;\n", key.c_str());
                    clearString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetValue(js, \"%s\", %s, %s);\n", key.c_str(), key.c_str(), key.c_str());
                    fromString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\njs[\"%s\"] = %s;\n", key.c_str(), key.c_str());
                    toString += buf;

                    // OS_LOGV("%s", output.c_str());
                    return;
                }
                if (value.is_number_integer())
                {
                    snprintf(buf, sizeof(buf), "//\nint64_t %s;\n", key.c_str());
                    eleString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\n%s = 0;\n", key.c_str());
                    clearString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetValue(js, \"%s\", %s, %s);\n", key.c_str(), key.c_str(), key.c_str());
                    fromString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\njs[\"%s\"] = %s;\n", key.c_str(), key.c_str());
                    toString += buf;

                    // OS_LOGV("%s", output.c_str());
                    return;
                }
                if (value.is_string())
                {
                    std::string c, ns, cn;
                    //
                    c = getRefClassFromString(value.get<std::string>(), ns, cn);
                    if (c.empty())
                    {
                        snprintf(buf, sizeof(buf), "//\nstd::string %s;\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetString(js, \"%s\", %s, %s);\n", key.c_str(), key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\njs[\"%s\"] = %s;\n", key.c_str(), key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    } else
                    {
                        //
                        refClass.push_back(c);
                        //
                        snprintf(buf, sizeof(buf), "//\n%s %s;\n", c.c_str(), key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= %s.fromJson(js) >=0;\n", key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\njs[\"%s\"] = %s.toJson();\n", key.c_str(), key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                }
                if (value.is_object())
                {
                    auto tmpCn      = input_cn + std::string("_Scl_") + key;
                    auto subObjName = parent_ns.empty() ? tmpCn : parent_ns + "::" + tmpCn;
                    //
                    JsonClassApiHelper tmph;
                    //
                    tmph.js = value;
                    leftClass.push_front({subObjName, tmph});

                    //
                    snprintf(buf, sizeof(buf), "//\n%s %s;\n", subObjName.c_str(), key.c_str());
                    eleString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                    clearString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\nmIsValid |= %s.fromJson(js) >=0;\n", key.c_str());
                    fromString += buf;
                    //
                    snprintf(buf, sizeof(buf), "//\njs[\"%s\"] = %s.toJson();\n", key.c_str(), key.c_str());
                    toString += buf;

                    // OS_LOGV("%s", output.c_str());
                    return;
                }
                if (value.is_array())  // nlohmannUserArrayContainer
                {
                    if (value.empty())
                    {
                        snprintf(buf, sizeof(buf), "//\nUNUSED_JS_ARR_KEY(%s);\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// clear (%s) \n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// extract (%s) \n", key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// tojs (%s) \n", key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    bool bRef = false;
                    for (auto& el_of_arr : value)
                    {
                        if (!el_of_arr.is_string())
                        {
                            continue;
                        }
                        std::string c, ns, cn;
                        //
                        c = getRefClassFromString(el_of_arr.get<std::string>(), ns, cn);
                        if (c.empty())
                        {
                            continue;
                        }
                        //
                        refClass.push_back(c);
                        //
                        snprintf(buf, sizeof(buf), "//\nnlohmannUserArrayContainer(%s) %s;\n", c.c_str(), key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetArrayClass(js, \"%s\", %s) > 0;\n", key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf,
                                 sizeof(buf),
                                 "//\n"
                                 "{\n"
                                 "auto tmpArr = nlohmann::json::array();\n"
                                 "  for(auto&el:%s) {\n"
                                 "     tmpArr.push_back(el.toJson());\n"
                                 "  }\n"
                                 "js[\"%s\"] = tmpArr;\n"
                                 "}",
                                 key.c_str(),
                                 key.c_str());
                        toString += buf;

                        //
                        bRef = true;
                        break;
                    }
                    if (bRef)
                    {
                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    //
                    auto pel = value.begin();
                    if (pel == value.end())
                    {
                        snprintf(buf, sizeof(buf), "//\nUNUSED_JS_ARR_KEY(%s);\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// clear (%s) \n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// extract (%s) \n", key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// tojs (%s) \n", key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }

                    if (pel->is_null())
                    {
                        snprintf(buf, sizeof(buf), "//\nUNUSED_JS_ARR_KEY(%s);\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// clear (%s) \n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// extract (%s) \n", key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf, sizeof(buf), "// tojs (%s) \n", key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    if (pel->is_boolean())
                    {
                        snprintf(buf, sizeof(buf), "//\nnlohmannUserArrayContainer(bool) %s;\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetArrayBool(js, \"%s\", %s) > 0;\n", key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf,
                                 sizeof(buf),
                                 "//\n"
                                 "js[\"%s\"] = %s;\n",
                                 key.c_str(),
                                 key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    if (pel->is_number_float())
                    {
                        snprintf(buf, sizeof(buf), "//\nnlohmannUserArrayContainer(double) %s;\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetArrayFloat(js, \"%s\", %s) > 0;\n", key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf,
                                 sizeof(buf),
                                 "//\n"
                                 "js[\"%s\"] = %s;\n",
                                 key.c_str(),
                                 key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    if (pel->is_number_integer())
                    {
                        snprintf(buf, sizeof(buf), "//\nnlohmannUserArrayContainer(int64_t) %s;\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetArrayInt(js, \"%s\", %s) > 0;\n", key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf,
                                 sizeof(buf),
                                 "//\n"
                                 "js[\"%s\"] = %s;\n",
                                 key.c_str(),
                                 key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    if (pel->is_string())
                    {
                        snprintf(buf, sizeof(buf), "//\nnlohmannUserArrayContainer(std::string) %s;\n", key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetArrayString(js, \"%s\", %s) > 0;\n", key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf,
                                 sizeof(buf),
                                 "//\n"
                                 "js[\"%s\"] = %s;\n",
                                 key.c_str(),
                                 key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                    if (pel->is_object())
                    {
                        auto tmpCn      = input_cn + std::string("_Scle_") + key;
                        auto subObjName = parent_ns.empty() ? tmpCn : parent_ns + "::" + tmpCn;
                        //
                        JsonClassApiHelper tmph;
                        //
                        tmph.js = value;
                        leftClass.push_front({subObjName, tmph});

                        //
                        snprintf(buf, sizeof(buf), "//\nnlohmannUserArrayContainer(%s) %s;\n", subObjName.c_str(), key.c_str());
                        eleString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\n%s.clear();\n", key.c_str());
                        clearString += buf;
                        //
                        snprintf(buf, sizeof(buf), "//\nmIsValid |= _JsonGetArrayClass(js, \"%s\", %s) > 0;\n", key.c_str(), key.c_str());
                        fromString += buf;
                        //
                        snprintf(buf,
                                 sizeof(buf),
                                 "//\n"
                                 "{\n"
                                 "auto tmpArr = nlohmann::json::array();\n"
                                 "  for(auto&el:%s) {\n"
                                 "     tmpArr.push_back(el.toJson());\n"
                                 "  }\n"
                                 "js[\"%s\"] = tmpArr;\n"
                                 "}",
                                 key.c_str(),
                                 key.c_str());
                        toString += buf;

                        // OS_LOGV("%s", output.c_str());
                        return;
                    }
                }
                OS_LOGW("unhandle, key=%s, value=%s", key.c_str(), value.dump().c_str());
            };
            //
            if (js.is_null())
            {
                // OS_LOGV("%s", output.c_str());
            } else if (js.is_object())
            {
                // OS_LOGV("%s", output.c_str());
                for (auto& [key, value] : js.items())
                {
                    hdl_keyval(key, value);
                }
            } else if (js.is_array())
            {
                // OS_LOGV("%s", output.c_str());
                hdl_keyval("array", js);
            } else
            {
                OS_LOGW("unhandle, input<%s,%s>", input_name.c_str(), js.dump().c_str());
            }

            output += eleString;
            //
            output += "//\n";
            output += "void _clear() override {\n";
            output += clearString;
            output += "}\n";
            //
            output += "//\n";
            output += "int _fromJson(const nlohmann::json& js) override {\n";
            output += fromString;
            output += "//\n";
            output += "return isValid()?0:-1;\n";
            output += "}\n";
            //
            output += "//\n";
            output += "nlohmann::json _toJson() const override {\n";
            output += "//\n";
            output += "auto js = nlohmann::json::object();\n";
            output += "//\n";
            output += toString;
            output += "//\n";
            output += "return js;\n";
            output += "}\n";
            //
            output += "//\n";
            output += "virtual bool _isValid() const override {\n";
            output += "return mIsValid;\n";
            output += "}\n";

            //
            output += "\n";
            output += "}; // class ";
            output += input_cn;
            // output += "\n";
            //
            output += "\n";
            output += "} // namespace ";
            output += parent_ns;
            output += "\n";
            //
            output += "\n";
            return output;
        }
    };

    static std::string Json2Class(const std::pair<std::string, JsonToClassFactory::JsonClassApiHelper>& input, std::list<std::string>& refClass)
    {
        //
        std::list<std::pair<std::string, JsonToClassFactory::JsonClassApiHelper>> leftClass;
        //
        leftClass.push_back(input);
        //
        std::string parent_ns;
        //
        std::list<std::string> genStr;

        while (!leftClass.empty())
        {
            auto el = leftClass.front();
            leftClass.pop_front();
            auto str = JsonToClassFactory::JsonClassApiHelper::Json2ClassOneLevel(el, leftClass, refClass, parent_ns);
            //
            genStr.push_back(str);
        }

        //
        std::string output;

        for (auto ritr = genStr.rbegin(); ritr != genStr.rend(); ++ritr)
        {
            output += *ritr;
        }
        output += "\n";

        return output;
    }


    static std::unordered_map<std::string, std::pair<std::string, JsonClassApiHelper>> globJsonClass(const std::string& path)
    {
        //
        std::unordered_map<std::string, std::pair<std::string, JsonClassApiHelper>> data;
        //
        auto jsFiles = glob_json_file(path);
        //
        for (const auto& f : jsFiles)
        {
            auto fp = path;
            //
            fp += "/";
            fp += f;
            //
            std::string ns, cn;
            //
            auto c = path2class(fp, ns, cn);
            //
            if (c.empty())
            {
                continue;
            }
            //
            JsonClassApiHelper h;
            //
            h.fromJsonFile(fp);
            //
            if (!h.isValid())
            {
                continue;
            }

            //
            data[f] = {c, h};
        }
        return data;
    }
};


#endif  // __JSON2CLASS_H__
