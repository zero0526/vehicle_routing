#pragma once
#include <map>
#include <string>
#include <vector>
class ET
{
private:
    std::string tag;
    std::map<std::string, std::string> attrs;
    std::string content;
    std::vector<ET *> children;

public:
    std::string getTag() const { return tag; }
    std::string getContent() const { return content; }
    std::string getData(const std::string &key) const
    {
        auto it = attrs.find(key);
        if (it != attrs.end())
            return it->second;
        return "";
    }
    std::vector<ET *> getChildren() const { return children; }
    ET *findNode(const std::string &tagName,
                 const std::map<std::string, std::string> &requiredData = {}) const;
    std::vector<ET *> findAll(const std::string &tagName,
                              const std::map<std::string, std::string> &requiredData = {}) const;
    void loadXML(const std::string &xmlString);
    ET(const std::string &t) : tag(t) {}
    ~ET()
    {
        for (auto child : children)
            delete child;
    }
};
