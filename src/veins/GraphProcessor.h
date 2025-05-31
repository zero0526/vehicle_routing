#pragma once
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "XMLProcessor.h"
class GraphProcessor
{
    public:
        GraphProcessor(
            const std::set<std::string> nodes, 
            const std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> edges,
            const std::unordered_map<std::string, std::vector<std::string>> toNodes
        ): nodes(nodes), edges(edges), toNodes(toNodes){}
        void findShortestPath(const std::string& source, const std::string& target);
        std::vector<std::string> shortestPath(std::string source, std::string target);
        float cheapCost(std::string source, std::string target);
        std::vector<std::string> toEdge(std::vector<std::string> nodes);
    private:
        std::set<std::string> nodes;
        std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> edges;
        std::unordered_map<std::string, std::vector<std::string>> toNodes;
        std::unordered_map<std::pair<std::string, std::string>, std::pair<std::vector<std::string>, float>, pair_hash> results; 
};