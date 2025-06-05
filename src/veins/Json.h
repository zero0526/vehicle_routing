#pragma once
#include <map>
#include <string>

class Json
{
private:
    std::map<std::string, std::string> jsonMap;

    std::string trim(const std::string &s) const;
    std::string unescapeString(const std::string &s) const;
    std::string escapeString(const std::string &s) const;

public:
    void add(const std::string &key, const std::string &value);
    std::string get(const std::string &key) const;
    std::string toJson() const;
    void parseJson(const std::string &jsonString);
    bool validJson(const std::string &jsonString);
};


