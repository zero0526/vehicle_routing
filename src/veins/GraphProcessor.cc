#include "GraphProcessor.h"
#include <queue>
#include <limits>
#include <unordered_map>
#include<algorithm>
void GraphProcessor::findShortestPath(const std::string& source, const std::string& target) {
    std::unordered_map<std::string, float> dist;
    std::unordered_map<std::string, std::string> prev;

    for (const auto& node : nodes) {
        dist[node] = std::numeric_limits<float>::infinity();
    }
    dist[source] = 0.0f;

    using Pair = std::pair<float, std::string>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq;
    pq.emplace(0.0f, source);

    while (!pq.empty()) {
        auto [currentDist, u] = pq.top(); pq.pop();
        if (u == target) break;
        if (!toNodes.count(u)) continue;

        for (const std::string& v : toNodes[u]) {
            auto it = edges.find({u, v});
            if (it == edges.end()) continue;

            float cost = currentDist + it->second.len;
            if (cost < dist[v]) {
                dist[v] = cost;
                prev[v] = u;
                pq.emplace(cost, v);
            }
        }
    }

    std::vector<std::string> path;
    if (dist[target] == std::numeric_limits<float>::infinity()) {
        results[{source, target}] = {{}, std::numeric_limits<float>::infinity()};
        return;
    }

    std::string current = target;
    while (current != source) {
        path.push_back(current);
        current = prev[current];
    }
    path.push_back(source);
    std::reverse(path.begin(), path.end());

    results[{source, target}] = {path, dist[target]};
}

std::vector<std::string> GraphProcessor::toEdge(std::vector<std::string> nodes){
    std::vector<std::string> path;
    for(int i=1; i<nodes.size();i++){
        path.push_back(this->edges[{nodes[i-1], nodes[i]}].id);
    }
    return path;
}
float GraphProcessor::cheapCost(std::string source, std::string target) {
    auto it = results.find({source, target});
    if (it == results.end()) {
        findShortestPath(source, target);
    }

    return results[{source, target}].second;
}

std::vector<std::string> GraphProcessor::shortestPath(std::string source, std::string target) {
    auto it = results.find({source, target});
    if (it == results.end()) {
        findShortestPath(source, target);
    }

    return toEdge(results[{source, target}].first);
}
