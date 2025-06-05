#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include "XMLProcessor.h"
#include "TaskGenerator.h"

struct AssignmentResult
{
    std::string source;
    float departureTime;
    std::vector<std::string> path;
    TimeWindow timeWindow;
};
struct PathInfo
{
    std::vector<std::string> path;
    float deviation;
    bool valid;
};
struct PairIHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        std::size_t hash1 = std::hash<T1>{}(pair.first);
        std::size_t hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ (hash2 << 1); // Kết hợp hai giá trị băm
    }
};
class PairAssigner
{
public:
    PairAssigner(const std::vector<std::pair<std::string,float>> &sources,
                 const std::unordered_map<std::string, TimeWindow> &targets,
                 const std::unordered_map<std::string, std::pair<float, float>> edges,
                 TaskGenerator &taskGenerator):        sources(sources),
        targets(targets), edges(edges),  taskGen(taskGenerator) {};

    std::vector<AssignmentResult> assign();

private:
    float calculateDeviation(const float arrival, const TimeWindow &window);
    std::vector<std::vector<float>> buildCostMatrix();
    std::vector<int> solveHungarian(const std::vector<std::vector<float>> &costMatrix);

private:
    std::vector<std::pair<std::string,float>> sources;
    std::unordered_map<std::string, TimeWindow> targets;
    std::unordered_map<std::string, std::pair<float, float>> edges;
    std::unordered_map<std::pair<int,int>, std::pair<std::vector<std::string>, float>,PairIHash> pathCache;
    TaskGenerator &taskGen;
};
