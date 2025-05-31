#pragma once
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "XMLProcessor.h"
#include <limits>

struct DijkstraResult
{
    std::unordered_map<std::string, float> dist;
    std::unordered_map<std::string, std::string> prev;
};

class TaskGenerator
{

public:
    TaskGenerator(
        const std::set<std::string> nodes,
        const std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> edges,
        const std::unordered_map<std::string, std::vector<std::string>> toNodes) : nodes(nodes), edges(edges), toNodes(toNodes) {}
    std::unordered_map<std::string, std::pair<float, float>> genTImeWindow(int n);
    std::vector<std::vector<std::string>> kShortestPath(std::string source, std::string target, int k);
    bool feasible(std::vector<std::string> sources, std::vector<std::string> targets);
    bool canAssign(std::string source, std::string target);
    float pathCost(std::vector<std::string> path)
    {
        float cost = 0;
        for (int i = 1; i < path.size(); i++)
        {
            auto it = this->edges.at({path[i - 1], path[i]});
            cost += it.len / it.speed;
        }
        if(!cost)
            return std::numeric_limits<float>::infinity();
        return cost;
    }

private:
    const std::set<std::string> nodes;
    const std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> edges;
    const std::unordered_map<std::string, std::vector<std::string>> toNodes;
    DijkstraResult dijkstra(const std::string& source,
                            const std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash>& customEdges);

    std::vector<std::string> reconstructPath(const std::string& source, const std::string& target,
                                             const std::unordered_map<std::string, std::string>& prev);
};