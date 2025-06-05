#include "GraphProcessor.h"
#include <queue>
#include <limits>
#include <unordered_set>
#include <algorithm>
#include <iostream>

std::pair<std::pair<std::string, std::string>, std::pair<std::vector<std::string>, float>> GraphProcessor::findShortestPath(const std::string &source, const std::string &target)
{
    using Pair = std::pair<float, std::string>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<>> pq;
    std::unordered_map<std::string, float> dist;
    std::unordered_map<std::string, std::string> prev;

    for (const auto &[edgeId, edgesVec] : toEdges)
    {
        dist[edgeId] = std::numeric_limits<float>::infinity();
    }
    dist[source] = 0.0;
    pq.emplace(0.0, source);

    while (!pq.empty())
    {
        auto [currCost, u] = pq.top();
        pq.pop();
        if (currCost > dist[u])
            continue;
        if (u == target)
            continue;

        if (toEdges.count(u))
        {
            for (const Edge &edge : toEdges[u])
            {
                std::string v = edge.id;
                float weight = edge.len / (edge.speed > 0 ? edge.speed : 1.0);
                float alt = dist[u] + weight;
                if (alt < dist[v])
                {
                    dist[v] = alt;
                    prev[v] = u;
                    pq.emplace(alt, v);
                }
            }
        }
    }

    if (dist[target] < std::numeric_limits<float>::infinity())
    {
        std::vector<std::string> path;
        std::string at = target;
        while (at != source)
        {
            path.push_back(at);
            at = prev[at];
        }
        path.push_back(source);
        std::reverse(path.begin(), path.end());
        results[{source, target}] = {path, dist[target]};
        return{
            {source, target},
            results[{source, target}]
        };
    }else{
        return {
            {source, target},
            {{}, std::numeric_limits<float>::infinity()}
        };
    }
}

std::vector<std::string> GraphProcessor::shortestPath(std::string source, std::string target)
{
    auto key = std::make_pair(source, target);
    if (results.find(key) != results.end())
    {
        return results[key].first;
    }
    auto [krs, vrs] = findShortestPath(source, target);
    return vrs.first;
}

float GraphProcessor::cheapCost(std::string source, std::string target)
{
    auto key = std::make_pair(source, target);
    if (results.find(key) != results.end())
    {
        return results[key].second;
    }
    auto [krs, vrs] = findShortestPath(source, target);
    return vrs.second;
}
