#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include "ET.h"

struct Edge
{
    std::string id;
    float len, speed;
    Edge(std::string id, float len, float speed) : id(id), len(len), speed(speed) {}
    Edge(std::string id) : id(id), len(0), speed(0) {}
    Edge() : id(""), len(0), speed(0) {}
};

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        auto h1 = std::hash<T1>()(p.first);
        auto h2 = std::hash<T2>()(p.second);
        return h1 ^ (h2 << 1);
    }
};

class XMLProcessor
{
public:
    XMLProcessor(const std::string &filePath);
    std::set<std::string> getNodes() const { return nodes; }
    std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> getEdges() const { return edges; }
    std::unordered_map<std::string, std::vector<std::string>> getFromNodes() const { return fromNodes; }
    std::unordered_map<std::string, std::vector<std::string>> getToNodes() const { return toNodes; }

private:
    std::set<std::string> nodes;
    std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> edges;
    std::unordered_map<std::string, std::vector<std::string>> fromNodes;
    std::unordered_map<std::string, std::vector<std::string>> toNodes;
};