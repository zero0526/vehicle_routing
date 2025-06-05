#include <vector>
#include "TaskGenerator.h"
#include "XMLProcessor.h"
#include <algorithm>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <functional>
#include <limits>
#include <sstream>
#include <iostream>

double getRandomDouble(double min, double max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen);
}

std::unordered_map<std::string, TimeWindow> TaskGenerator::genTimeWindow(int n)
{
    std::unordered_set<std::string> edges;
    float minTime = std::numeric_limits<float>::max();
    float maxTime = std::numeric_limits<float>::min();
    for (const auto &[from, toVec] : fromEdges)
    {
        edges.insert(from);
        for (const auto &edge : toVec)
        {
            edges.insert(edge.id);
            minTime = std::min(minTime, edge.len / edge.speed);
            maxTime = std::max(maxTime, edge.len / edge.speed);
        }
    }
    std::vector<std::string> edgeVector(edges.begin(), edges.end());
    std::random_device rd;
    std::mt19937 gen(rd());

    std::shuffle(edgeVector.begin(), edgeVector.end(), gen);
    edgeVector.resize(std::min(n, static_cast<int>(edgeVector.size())));

    std::unordered_map<std::string, TimeWindow> timeWindows;
    for (const auto &edge : edgeVector)
    {
        float earlyTime = getRandomDouble(1 * maxTime, 2 * maxTime);

        float extraTime = getRandomDouble(minTime, 2 * minTime);
        float lateTime = earlyTime + extraTime;

        timeWindows[edge] = {earlyTime, lateTime};
    }
    return timeWindows;
}


std::vector<std::string> reconstructNodePath(
    const std::unordered_map<std::string, std::string> &prev,
    const std::string &source,
    const std::string &target)
{
    std::vector<std::string> path;
    std::string current = target;
    while (prev.count(current) || current == source)
    {
        path.push_back(current);
        if (current == source)
            break;
        if (!prev.count(current))
        {
            return {};
        }
        current = prev.at(current);
    }
    if (path.empty() || path.back() != source)
        return {};
    std::reverse(path.begin(), path.end());
    return path;
}

float calculate_current_penalty(float arrivalTime, const TimeWindow &tw)
{
    if (arrivalTime == std::numeric_limits<float>::infinity())
    {
        return std::numeric_limits<float>::infinity();
    }
    float penalty = 0.0f;
    if (arrivalTime < tw.earlyTime)
    {
        penalty = tw.earlyTime - arrivalTime;
    }
    else if (arrivalTime > tw.lateTime)
    {
        penalty = arrivalTime - tw.lateTime;
    }
    return penalty;
}

std::string TaskGenerator::joinPath(const std::vector<std::string> &path)
{
    std::ostringstream oss;
    for (const auto &node : path)
    {
        oss << node << " ";
    }
    return oss.str();
}
float TaskGenerator::timeFunc(std::vector<std::string>path){
    float total_time = 0.0f;
    for (const auto &edge : path)
    {
        auto it = edges.find(edge);
        if (it != edges.end())
        {
            total_time += it->second.first / it->second.second; // len / speed
        }
    }
    return total_time;
}

std::pair<std::vector<std::string>, float> TaskGenerator::findBestTimeWindowPath(
    std::string source,
    std::pair<std::string, TimeWindow> target_info)
{
    auto [_, shortest_result] = graphProcessor.findShortestPath(source, target_info.first);
    auto& [initial_path, initial_arrival_time] = shortest_result;

    if (initial_arrival_time == std::numeric_limits<float>::infinity()) {
        return {{}, initial_arrival_time};
    }

    if (initial_arrival_time >= target_info.second.earlyTime) {
        return {initial_path, initial_arrival_time};
    }

    float best_penalty = calculate_current_penalty(initial_arrival_time, target_info.second);
    float best_time = initial_arrival_time;
    std::vector<std::string> best_path = initial_path;

    const int max_attempts = 3;
    int limit = std::min<int>(static_cast<int>(initial_path.size()) - 1, max_attempts);

    for (int i = 1; i <= limit; ++i) {
        size_t idx = initial_path.size() - i;
        const std::string& current_edge = initial_path[idx];

        auto edge_it = edges.find(current_edge);
        if (edge_it == edges.end() || edge_it->second.second == 0.0f) {
            continue; // Skip if edge not found or speed is zero
        }

        std::vector<std::string> prefix_path(initial_path.begin(), initial_path.begin() + idx);
        if (prefix_path.empty()) break;

        const std::string& last_edge = prefix_path.back();
        auto to_it = toEdges.find(last_edge);
        if (to_it == toEdges.end()) continue;

        for (const Edge& next_edge : to_it->second) {
            if (next_edge.id == current_edge) continue;

            auto [__, alt_result] = graphProcessor.findShortestPath(next_edge.id, target_info.first);
            auto& [alt_path, alt_arrival] = alt_result;
            if (alt_arrival == std::numeric_limits<float>::infinity()) continue;

            // Reserve capacity to avoid reallocation
            std::vector<std::string> candidate_path;
            candidate_path.reserve(prefix_path.size() + alt_path.size());
            candidate_path.insert(candidate_path.end(), prefix_path.begin(), prefix_path.end());
            candidate_path.insert(candidate_path.end(), alt_path.begin(), alt_path.end());

            float candidate_time = timeFunc(candidate_path);
            float penalty = calculate_current_penalty(candidate_time, target_info.second);

            if (penalty < best_penalty) {
                best_penalty = penalty;
                best_time = candidate_time;
                best_path = std::move(candidate_path);

                if (penalty == 0.0f) {
                    return {best_path, best_time}; // Found perfect match
                }
            }
        }
    }

    return {best_path, best_time};
}


std::unordered_map<float, std::vector<std::string>> TaskGenerator::findKShortestPaths(
    const std::string &sourceId,
    const std::string &targetId,
    int k)
{
    struct Path {
        std::vector<std::string> nodes;
        float total_length;

        bool operator>(const Path &other) const {
            return total_length > other.total_length;  // Min-Heap
        }
    };

    auto dijkstra = [&](const std::string &start, const std::string &end) -> Path {
        std::priority_queue<Path, std::vector<Path>, std::greater<Path>> pq;
        pq.push({{start}, 0.0f});

        std::unordered_set<std::string> visited;

        while (!pq.empty()) {
            Path path = pq.top();
            pq.pop();
            std::string current = path.nodes.back();

            if (current == end) {
                return path;
            }

            if (visited.count(current)) continue;
            visited.insert(current);

            if (toEdges.find(current) == toEdges.end()) continue;

            for (const Edge &edge : toEdges.at(current)) {
                std::string next = edge.id;
                float len = edge.len;

                std::vector<std::string> newPath = path.nodes;
                newPath.push_back(next);
                pq.push({newPath, path.total_length + len});
            }
        }

        return {{}, std::numeric_limits<float>::max()}; // Không tìm thấy đường đi
    };

    std::vector<Path> shortestPaths;
    std::set<std::vector<std::string>> pathSet;

    Path firstPath = dijkstra(sourceId, targetId);
    if (firstPath.nodes.empty()) return {};

    shortestPaths.push_back(firstPath);
    pathSet.insert(firstPath.nodes);

    std::priority_queue<Path, std::vector<Path>, std::greater<Path>> candidates;

    for (int kIdx = 1; kIdx < k; ++kIdx) {
        const Path &lastPath = shortestPaths[kIdx - 1];

        for (size_t i = 0; i < lastPath.nodes.size() - 1; ++i) {
            std::string spurNode = lastPath.nodes[i];
            std::vector<std::string> rootPath(lastPath.nodes.begin(), lastPath.nodes.begin() + i + 1);

            std::vector<Edge> removedEdges;

            for (const Path &p : shortestPaths) {
                if (p.nodes.size() > i && std::equal(rootPath.begin(), rootPath.end(), p.nodes.begin())) {
                    std::string from = p.nodes[i];
                    std::string to = p.nodes[i + 1];

                    if (toEdges.find(from) == toEdges.end()) continue;

                    auto &edgesFrom = toEdges[from];
                    auto it = std::remove_if(edgesFrom.begin(), edgesFrom.end(), [&](const Edge &e) {
                        return e.id == to;
                    });

                    for (auto remIt = it; remIt != edgesFrom.end(); ++remIt) {
                        removedEdges.push_back(*remIt);
                    }

                    edgesFrom.erase(it, edgesFrom.end());
                }
            }

            Path spurPath = dijkstra(spurNode, targetId);

            for (const Edge &e : removedEdges) {
                toEdges[spurNode].push_back(e);
            }

            if (!spurPath.nodes.empty()) {
                std::vector<std::string> totalPath = rootPath;
                totalPath.insert(totalPath.end(), spurPath.nodes.begin() + 1, spurPath.nodes.end());

                if (pathSet.count(totalPath)) continue;

                float totalLength = 0.0f;
                for (size_t j = 0; j < totalPath.size() - 1; ++j) {
                    if (toEdges.find(totalPath[j]) == toEdges.end()) {
                        totalLength = std::numeric_limits<float>::max();
                        break;
                    }
                    const auto &edges = toEdges.at(totalPath[j]);
                    for (const Edge &e : edges) {
                        if (e.id == totalPath[j + 1]) {
                            totalLength += e.len;
                            break;
                        }
                    }
                }

                Path newPath = {totalPath, totalLength};
                candidates.push(newPath);
                pathSet.insert(totalPath);
            }
        }

        if (candidates.empty()) break;

        shortestPaths.push_back(candidates.top());
        candidates.pop();
    }

    std::unordered_map<float, std::vector<std::string>> result;
    for (const auto &path : shortestPaths) {
        result[path.total_length] = path.nodes;
    }

    return result;
}
bool TaskGenerator::feasible(std::vector<std::string> sources, std::vector<std::string> targets) {
    if (sources.size() != targets.size()) {
        return false; // Số lượng sources và targets không khớp
    }

    std::unordered_map<std::string, bool> usedTargets; // Đánh dấu các targets đã được sử dụng

    for (const auto& source : sources) {
        bool foundMatch = false;

        for (const auto& target : targets) {
            if (usedTargets[target]) {
                continue; // Target này đã được sử dụng
            }

            auto pathResult= graphProcessor.shortestPath(source, target);
            if (!pathResult.empty()) {
                usedTargets[target] = true; \
                foundMatch = true;
                break;
            }
        }

        if (!foundMatch) {
            return false;
        }
    }

    return true;
}
