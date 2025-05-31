#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

struct EdgeInfo {
    float length;
    float speed;
    EdgeInfo() : length(0), speed(0) {}
    EdgeInfo(float length, float speed) : length(length), speed(speed) {}
};

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

class DijkstraPath {

private:
    std::unordered_map<std::string, std::vector<std::string>> toEdge;
    std::unordered_map<std::string, EdgeInfo> edge_dict;
    std::unordered_map<std::pair<std::string, std::string>, std::pair<std::vector<std::string>, float>, pair_hash> rs;

public:
    DijkstraPath() {}
    DijkstraPath(std::unordered_map<std::string, std::vector<std::string>> toEdge,
        std::unordered_map<std::string, EdgeInfo> edge_dict) : toEdge(toEdge), edge_dict(edge_dict) {}
    ~DijkstraPath() {
        toEdge.clear();
        edge_dict.clear();
        rs.clear();
    }
    std::pair<std::vector<std::string>, float> findShortest(
        const std::string& source,
        const std::string& target
    );
    std::vector<std::string> findShortestPath(
        const std::string& source,
        const std::string& target
    );
    float CalcuShortestPath(
        const std::string& source,
        const std::string& target
    );
};
