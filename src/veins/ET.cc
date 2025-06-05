#include "ET.h"
#include <fstream>
#include <sstream>
#include <stack>
#include <iostream>
#include <cctype>
#include <regex>
void ET::loadXML(const std::string &xmlPath)
{
    std::ifstream file(xmlPath);
    if (!file)
    {
        std::cerr << "Cannot open this file: " << xmlPath << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string xmlString = buffer.str();

    std::stack<ET *> nodeStack;
    nodeStack.push(this);

    size_t pos = 0;
    while (pos < xmlString.size())
    {
        size_t lt = xmlString.find('<', pos);
        if (lt == std::string::npos)
            break;

        if (lt > pos)
        {
            std::string text = xmlString.substr(pos, lt - pos);
            size_t start = text.find_first_not_of(" \t\n\r");
            size_t end = text.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos)
            {
                std::string trimmed = text.substr(start, end - start + 1);
                if (!trimmed.empty())
                {
                    nodeStack.top()->content += trimmed + ",";
                }
            }
        }

        size_t gt = xmlString.find('>', lt);
        if (gt == std::string::npos)
            break;

        std::string tagContent = xmlString.substr(lt + 1, gt - lt - 1);
        pos = gt + 1;

        if (!tagContent.empty() && tagContent[0] == '/')
        {
            if (!nodeStack.empty())
                nodeStack.pop();
        }
        else
        {
            bool selfClosing = false;
            if (!tagContent.empty() && tagContent.back() == '/')
            {
                selfClosing = true;
                tagContent.pop_back();
            }

            // std::istringstream iss(tagContent);
            // std::string tagName;
            // iss >> tagName;

            // ET *node = new ET(tagName);

            // std::string attr;
            // while (iss >> attr)
            // {
            //     size_t eq = attr.find('=');
            //     if (eq != std::string::npos)
            //     {
            //         std::string key = attr.substr(0, eq);
            //         std::string value = attr.substr(eq + 1);
            //         if (!value.empty() && value.front() == '\"' && value.back() == '\"')
            //             value = value.substr(1, value.size() - 2);
            //         node->attrs[key] = value;
            //     }
            // }

            // nodeStack.top()->children.push_back(node);

            // if (!selfClosing)
            //     nodeStack.push(node);
                        std::istringstream iss(tagContent);
            std::string tagName;
            iss >> tagName;

            ET *node = new ET(tagName);

            std::regex attrRegex(R"raw((\w+)\s*=\s*"(.*?)")raw");
            auto attrBegin = std::sregex_iterator(tagContent.begin(), tagContent.end(), attrRegex);
            auto attrEnd = std::sregex_iterator();
            for (auto it = attrBegin; it != attrEnd; ++it) {
                std::smatch match = *it;
                std::string key = match[1].str();
                std::string value = match[2].str();
                node->attrs[key] = value;
            }

            nodeStack.top()->children.push_back(node);

            if (!selfClosing)
                nodeStack.push(node);
        }
    }
}
ET *ET::findNode(const std::string &tagName,
                 const std::map<std::string, std::string> &requiredData) const
{
    if (this->tag == tagName)
    {
        bool match = true;
        for (const auto &[key, valueRegexStr] : requiredData)
        {
            auto it = attrs.find(key);
            if (it == attrs.end())
            {
                match = false;
                break;
            }

            std::regex valueRegex(valueRegexStr);

            if (!std::regex_match(it->second, valueRegex))
            {
                match = false;
                break;
            }
        }
        if (match)
            return const_cast<ET *>(this);
    }

    for (auto child : children)
    {
        ET *result = child->findNode(tagName, requiredData);
        if (result)
            return result;
    }

    return nullptr;
}

std::vector<ET *> ET::findAll(const std::string &tagName,
                              const std::map<std::string, std::string> &requiredData) const
{
    std::vector<ET *> results;

    if (this->tag == tagName)
    {
        bool match = true;
        for (const auto &[key, valueRegexStr] : requiredData)
        {
            auto it = attrs.find(key);
            if (it == attrs.end())
            {
                match = false;
                break;
            }

            std::regex valueRegex(valueRegexStr);

            if (!std::regex_match(it->second, valueRegex))
            {
                match = false;
                break;
            }
        }

        if (match)
            results.push_back(const_cast<ET *>(this));
    }

    for (auto child : children)
    {
        auto childResults = child->findAll(tagName, requiredData);
        results.insert(results.end(), childResults.begin(), childResults.end());
    }

    return results;
}
