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
    std::string target;
    std::vector<std::string> path;
    float deviation;
};
struct PathInfo
{
    std::vector<std::string> path;
    float deviation;
    bool valid;
};
class PairAssigner
{
public:
    using TimeWindow = std::pair<float, float>;

    PairAssigner(const std::vector<std::string> &sources,
                 const std::vector<std::string> &targets,
                 const std::unordered_map<std::string, TimeWindow> &targetTimeWindow,
                 TaskGenerator &taskGenerator);

    std::vector<AssignmentResult> assign(int k);

private:
    float calculateDeviation(const std::vector<std::string> &path, const TimeWindow &window);
    std::vector<std::vector<float>> buildCostMatrix(int k);
    float findBestPathCost(const std::string &source, const std::string &target, int k);
    std::vector<int> solveHungarian(const std::vector<std::vector<float>> &costMatrix);

private:
    std::vector<std::string> sources;
    std::vector<std::string> targets;
    std::unordered_map<std::string, TimeWindow> timeWindows;
    TaskGenerator &taskGen;
    std::unordered_map<std::string, std::unordered_map<std::string, PathInfo>> cachedPaths;
};