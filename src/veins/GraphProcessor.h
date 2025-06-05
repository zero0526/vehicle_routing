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
            const std::unordered_map<std::string, std::vector<Edge>> fromEdges,
            const std::unordered_map<std::string, std::vector<Edge>> toEdges
        ): fromEdges(fromEdges), toEdges(toEdges){}
        GraphProcessor(): fromEdges({}), toEdges({}) {}
        std::pair<std::pair<std::string, std::string>, std::pair<std::vector<std::string>, float>> findShortestPath(const std::string& source, const std::string& target);
        std::vector<std::string> shortestPath(std::string source, std::string target);
        float cheapCost(std::string source, std::string target);
    private:
        std::unordered_map<std::string, std::vector<Edge>> fromEdges;
        std::unordered_map<std::string, std::vector<Edge>> toEdges;
        std::unordered_map<std::pair<std::string, std::string>, std::pair<std::vector<std::string>, float>, pair_hash> results; 
};
