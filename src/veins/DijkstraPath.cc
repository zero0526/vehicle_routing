#include "DijkstraPath.h"
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>

std::pair<std::vector<std::string>, float> DijkstraPath::findShortest(
    const std::string& source,
    const std::string& target
) {
    if (edge_dict.find(source) == edge_dict.end() || edge_dict.find(target) == edge_dict.end()) {
        return {{}, -1};
    }

    std::unordered_map<std::string, float> dist;
    std::unordered_map<std::string, std::string> parent;

    for (const auto& kv : edge_dict) {
        dist[kv.first] = std::numeric_limits<float>::infinity();
    }
    dist[source] = 0.0f;

    using PQElem = std::pair<float, std::string>;
    std::priority_queue<PQElem, std::vector<PQElem>, std::greater<PQElem>> pq;
    pq.push({0.0f, source});

    while (!pq.empty()) {
        auto [cur_dist, u] = pq.top();
        pq.pop();

        if (cur_dist > dist[u]) continue;
        if (u == target) break;
        if (toEdge.find(u) == toEdge.end()) continue;

        for (const std::string& v : toEdge[u]) {
            if (edge_dict.find(v) == edge_dict.end()) continue;

            float cost = edge_dict[v].length;
            float new_dist = cur_dist + cost;

            if (new_dist < dist[v]) {
                dist[v] = new_dist;
                parent[v] = u;
                pq.push({new_dist, v});
            }
        }
    }

    if (dist[target] == std::numeric_limits<float>::infinity()) {
        rs[{source, target}] = {{}, -1};
        return {{}, -1};
    }

    std::vector<std::string> path;
    float total_cost = 0.0f;

    for (std::string at = target; at != source; at = parent[at]) {
        total_cost += edge_dict[at].length;
        path.push_back(at);
    }
    total_cost += edge_dict[source].length;
    path.push_back(source);
    std::reverse(path.begin(), path.end());

    rs[{source, target}] = {path, total_cost};

    for (size_t i = 1; i < path.size(); ++i) {
        std::vector<std::string> subpath(path.begin(), path.begin() + i + 1);
        float subcost = 0.0f;
        for (size_t j = 1; j <= i; ++j) {
            subcost += edge_dict[path[j]].length;
        }
        rs[{path[0], path[i]}] = {subpath, subcost};
    }

    return {path, total_cost};
}

std::vector<std::string> DijkstraPath::findShortestPath(
    const std::string& source,
    const std::string& target
) {
    if (rs.find({source, target}) == rs.end()) {
        auto [path, cost] = findShortest(source, target);
        if (cost == -1.0f) return {};
    }
    return rs[{source, target}].first;
}

float DijkstraPath::CalcuShortestPath(
    const std::string& source,
    const std::string& target
) {
    if (rs.find({source, target}) == rs.end()) {
        auto [path, cost] = findShortest(source, target);
        if (cost == -1.0f) return -1.0f;
    }
    return rs[{source, target}].second;
}
