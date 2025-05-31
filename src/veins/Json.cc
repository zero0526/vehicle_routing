#include "Json.h"
#include <sstream>
#include <stdexcept>
#include <cctype>

void Json::add(const std::string &key, const std::string &value)
{
    jsonMap[key] = value;
}

std::string Json::get(const std::string &key) const
{
    auto it = jsonMap.find(key);
    if (it != jsonMap.end())
    {
        return it->second;
    }
    return "";
}

std::string Json::trim(const std::string &s) const
{
    size_t start = 0, end = s.size();
    while (start < s.size() && isspace(s[start]))
        start++;
    while (end > start && isspace(s[end - 1]))
        end--;
    return s.substr(start, end - start);
}

std::string Json::escapeString(const std::string &s) const
{
    std::string result = "\"";
    for (char c : s)
    {
        switch (c)
        {
        case '\"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result += c;
            break;
        }
    }
    result += "\"";
    return result;
}

std::string Json::unescapeString(const std::string &s) const
{
    std::string result;
    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] == '\\' && i + 1 < s.length())
        {
            i++;
            switch (s[i])
            {
            case '\"':
                result += '\"';
                break;
            case '\\':
                result += '\\';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            default:
                result += s[i];
                break;
            }
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}

std::string Json::toJson() const
{
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto &pair : jsonMap)
    {
        if (!first)
            oss << ", ";
        oss << escapeString(pair.first) << ": " << escapeString(pair.second);
        first = false;
    }
    oss << "}";
    return oss.str();
}
bool Json::validJson(const std::string &jsonString){
    std::string s = trim(jsonString);
    if(s.front()!='{'||s.back()!='}'){
        return false;
    }else{
        return true;
    }
}
void Json::parseJson(const std::string &jsonString)
{
    jsonMap.clear();
    std::string s = trim(jsonString);
    if (s.front() != '{' || s.back() != '}')
    {
        throw std::runtime_error("Invalid JSON format");
    }
    s = s.substr(1, s.size() - 2); // remove { }

    size_t i = 0;
    while (i < s.size())
    {
        while (i < s.size() && isspace(s[i]))
            i++;
        if (s[i] != '\"')
            throw std::runtime_error("Expected '\"' at key start");

        size_t key_start = ++i;
        while (i < s.size() && s[i] != '\"')
            i++;
        std::string key = s.substr(key_start, i - key_start);
        i++; // skip ending quote

        while (i < s.size() && s[i] != ':')
            i++;
        i++; // skip colon

        while (i < s.size() && isspace(s[i]))
            i++;

        if (s[i] != '\"')
            throw std::runtime_error("Expected '\"' at value start");

        size_t val_start = ++i;
        while (i < s.size() && s[i] != '\"')
        {
            if (s[i] == '\\' && i + 1 < s.size())
                i++; // skip escape
            i++;
        }
        std::string value = s.substr(val_start, i - val_start);
        i++; // skip ending quote

        jsonMap[unescapeString(key)] = unescapeString(value);

        while (i < s.size() && isspace(s[i]))
            i++;
        if (i < s.size())
        {
            if (s[i] == ',')
                i++;
            else
                throw std::runtime_error("Expected ',' or '}'");
        }
    }
}
