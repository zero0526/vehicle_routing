#include "PairAssigner.h"
#include <limits>
#include <cmath>
#include <algorithm>
#include "XMLProcessor.h"
#include<iostream>
PairAssigner::PairAssigner(const std::vector<std::string> &src,
                           const std::vector<std::string> &tgt,
                           const std::unordered_map<std::string, TimeWindow> &windows,
                           TaskGenerator &generator)
    : sources(src), targets(tgt), timeWindows(windows), taskGen(generator) {
        for (auto[k, v] :windows){
            std::cout<< "Target: " << k << ", Time Window: [" << v.first << ", " << v.second << "]" << std::endl;
        }
    }

float PairAssigner::calculateDeviation(const std::vector<std::string> &path, const TimeWindow &window)
{
    float arrival = taskGen.pathCost(path);
    if (arrival < window.first)
        return window.first - arrival;
    if (arrival > window.second)
        return arrival - window.second;
    return 0.0f;
}

float PairAssigner::findBestPathCost(const std::string &source, const std::string &target, int k)
{
    if (!taskGen.canAssign(source, target))
    {
        cachedPaths[source][target] = {{}, std::numeric_limits<float>::infinity(), false};
        return std::numeric_limits<float>::infinity();
    }

    auto kPaths = taskGen.kShortestPath(source, target, k);
    float minDev = std::numeric_limits<float>::infinity();
    std::vector<std::string> bestPath;

    for (const auto &path : kPaths)
    {
        float dev = calculateDeviation(path, timeWindows.at(target));
        if (dev < minDev)
        {
            minDev = dev;
            bestPath = path;
        }
    }

    cachedPaths[source][target] = {bestPath, minDev, true};
    return minDev;
}

std::vector<std::vector<float>> PairAssigner::buildCostMatrix(int k)
{
    size_t n = sources.size();
    std::vector<std::vector<float>> costMatrix(n, std::vector<float>(n));
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            costMatrix[i][j] = findBestPathCost(sources[i], targets[j], k);
        }
    }
    return costMatrix;
}

std::vector<int> PairAssigner::solveHungarian(const std::vector<std::vector<float>> &costMatrix)
{
    size_t n = costMatrix.size();
    std::vector<int> u(n + 1), v(n + 1), p(n + 1), way(n + 1);

    for (size_t i = 1; i <= n; ++i)
    {
        p[0] = i;
        std::vector<float> minv(n + 1, std::numeric_limits<float>::infinity());
        std::vector<bool> used(n + 1, false);
        size_t j0 = 0;
        do
        {
            used[j0] = true;
            size_t i0 = p[j0], j1;
            float delta = std::numeric_limits<float>::infinity();
            for (size_t j = 1; j <= n; ++j)
            {
                if (!used[j])
                {
                    float cur = costMatrix[i0 - 1][j - 1] - u[i0] - v[j];
                    if (cur < minv[j])
                    {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta)
                    {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }
            for (size_t j = 0; j <= n; ++j)
            {
                if (used[j])
                {
                    u[p[j]] += delta;
                    v[j] -= delta;
                }
                else
                {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do
        {
            size_t j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    std::vector<int> result(n);
    for (size_t j = 1; j <= n; ++j)
        result[p[j] - 1] = j - 1;

    return result;
}

std::vector<AssignmentResult> PairAssigner::assign(int k)
{
    auto costMatrix = buildCostMatrix(k);
    auto assignment = solveHungarian(costMatrix);
    std::vector<AssignmentResult> result;

    for (size_t i = 0; i < assignment.size(); ++i)
    {
        const std::string &src = sources[i];
        const std::string &tgt = targets[assignment[i]];
        const auto &info = cachedPaths[src][tgt];

        if (info.valid)
        {
            result.push_back({src, tgt, info.path, info.deviation});
        }
        else
        {
            result.push_back({src, tgt, {}, std::numeric_limits<float>::infinity()});
        }
    }

    return result;
}
