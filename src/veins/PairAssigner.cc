#include "PairAssigner.h"
#include <limits>
#include <cmath>
#include <algorithm>
#include "XMLProcessor.h"
#include "HungarianAlgo.h"
#include<iostream>

float PairAssigner::calculateDeviation(const float arrival, const TimeWindow &window)
{
    if (arrival < window.earlyTime)
        return window.earlyTime - arrival;
    if (arrival > window.lateTime)
        return arrival - window.lateTime;
    return 0.0f;
}

std::vector<std::vector<float>> PairAssigner::buildCostMatrix()
{
    size_t n = sources.size();
    std::vector<std::vector<float>> costMatrix(n, std::vector<float>(n));

    for (size_t i = 0; i < n; ++i)
    {
        size_t j = 0;
        for (const auto& timeWindow : targets)
        {
            TimeWindow newtimeWindow ={timeWindow.second.earlyTime-sources[i].second, timeWindow.second.lateTime-sources[i].second};

            auto result = taskGen.findBestTimeWindowPath(sources[i].first, {timeWindow.first, newtimeWindow});
            costMatrix[i][j] = calculateDeviation(result.second, newtimeWindow);
            pathCache[{i, j}] = {result.first, result.second};
            ++j;
        }
    }

    return costMatrix;
}

std::vector<int> PairAssigner::solveHungarian(const std::vector<std::vector<float>> &costMatrix_param)
{
    if (costMatrix_param.empty()) {
        return {};
    }
    if (!costMatrix_param.empty() && costMatrix_param[0].empty()) {
        return std::vector<int>(costMatrix_param.size(), -1);
    }

    HungarianAlgo solver;
    return solver.solveHungarian(costMatrix_param);
}

std::vector<AssignmentResult> PairAssigner::assign()
{
    std::vector<AssignmentResult> results;

    std::vector<std::vector<float>> costMatrix = buildCostMatrix();
    std::vector<int> assignment = solveHungarian(costMatrix);
    if (assignment.empty()) {
        return results;
    }

    for (size_t i = 0; i < assignment.size(); ++i) {
        int targetIndex = assignment[i];
        if (targetIndex >= 0 && targetIndex < static_cast<int>(targets.size())) {
            auto targetIt = std::next(targets.begin(), targetIndex);
            const std::string& source = sources[i].first;
            const std::string& target = targetIt->first;
            const TimeWindow& timeWindow = {targetIt->second.earlyTime - sources[i].second, targetIt->second.lateTime - sources[i].second};

            auto pathCacheKey = std::make_pair(static_cast<int>(i), targetIndex);
            const std::vector<std::string>& path = pathCache[pathCacheKey].first;
            float deviation = calculateDeviation(pathCache[pathCacheKey].second, timeWindow);

            AssignmentResult result = {source, pathCache[pathCacheKey].second+sources[i].second, path, targetIt->second};
            results.push_back(result);
        }
    }

    return results;
}
