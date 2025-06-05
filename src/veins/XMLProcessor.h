#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include "ET.h"

struct Edge
{
    std::string id;
    float len, speed;
    Edge(std::string id, float len, float speed) : id(id), len(len), speed(speed) {}
    Edge(std::string id) : id(id), len(0), speed(0) {}
    Edge() : len(0), speed(0) {}
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
    std::unordered_map<std::string, std::vector<Edge>> getFromEdge() const { return fromEdge; }
    std::unordered_map<std::string, std::vector<Edge>> getToEdge() const { return toEdge; }
    std::unordered_map<std::string, std::pair<float, float>> getEdges() const { return edges; }
private:
    std::unordered_map<std::string, std::pair<float, float>> edges;
    std::unordered_map<std::string, std::vector<Edge>> fromEdge;
    std::unordered_map<std::string, std::vector<Edge>> toEdge;
};
