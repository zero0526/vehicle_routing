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

double getRandomDouble(double min, double max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen);
}

std::unordered_map<std::string, std::pair<float, float>> TaskGenerator::genTImeWindow(int n)
{
    float maxPeriod = 0.0;
    for (auto [k, v] : this->edges)
    {
        float t = v.len / v.speed;
        if (maxPeriod < t)
            maxPeriod = t;
    }
    int len = this->nodes.size();
    if (n > len || n == 0)
        return {};
    std::unordered_set<int> unduplicate;
    std::unordered_map<std::string, std::pair<float, float>> rs;
    std::vector<std::string> ns(this->nodes.begin(), this->nodes.end());
    while (n > rs.size())
    {
        int idx = getRandomDouble(1, len) - 1;
        while (unduplicate.find(idx) != unduplicate.end())
        {
            idx = (idx + 1) % len;
        }
        unduplicate.insert(idx);
        float var = getRandomDouble(maxPeriod / 4.0, maxPeriod / 2.0);
        float mean = getRandomDouble(maxPeriod / 2.0, maxPeriod * (len - 1) / 2.0);
        rs[ns[idx]] = {mean - var / 2.0, mean + var / 2.0};
    }
    return rs;
}

DijkstraResult TaskGenerator::dijkstra(
    const std::string &source,
    const std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> &customEdges)
{

    std::unordered_map<std::string, float> dist;
    std::unordered_map<std::string, std::string> prev;
    for (const auto &node : this->nodes)
        dist[node] = std::numeric_limits<float>::infinity();
    dist[source] = 0;

    auto cmp = [&dist](const std::string &a, const std::string &b)
    {
        return dist[a] > dist[b];
    };
    std::priority_queue<std::string, std::vector<std::string>, decltype(cmp)> pq(cmp);
    pq.push(source);

    while (!pq.empty())
    {
        std::string u = pq.top();
        pq.pop();
        if (this->toNodes.find(u) == this->toNodes.end())
            continue;
        for (const auto &v : this->toNodes.at(u))
        {
            auto it = customEdges.find({u, v});
            if (it == customEdges.end())
                continue;
            float len = it->second.len / it->second.speed;
            if (dist[v] > dist[u] + len)
            {
                dist[v] = dist[u] + len;
                prev[v] = u;
                pq.push(v);
            }
        }
    }
    return {dist, prev};
}

std::vector<std::string> TaskGenerator::reconstructPath(
    const std::string &source,
    const std::string &target,
    const std::unordered_map<std::string, std::string> &prev)
{

    std::vector<std::string> path;
    std::string cur = target;
    while (cur != source)
    {
        if (prev.find(cur) == prev.end())
            return {};
        path.push_back(cur);
        cur = prev.at(cur);
    }
    path.push_back(source);
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<std::vector<std::string>> TaskGenerator::kShortestPath(
    std::string source, std::string target, int k)
{

    std::vector<std::vector<std::string>> result;
    auto base = dijkstra(source, this->edges);
    auto firstPath = reconstructPath(source, target, base.prev);
    if (firstPath.empty())
        return result;
    result.push_back(firstPath);

    using Path = std::vector<std::string>;
    using PathEntry = std::pair<float, Path>;
    auto pathCost = [&](const Path &path)
    {
        float cost = 0;
        for (size_t i = 0; i + 1 < path.size(); ++i)
        {
            auto it = this->edges.find({path[i], path[i + 1]});
            if (it == this->edges.end())
                return std::numeric_limits<float>::infinity();
            cost += it->second.len / it->second.speed;
        }
        return cost;
    };

    auto cmp = [](const PathEntry &a, const PathEntry &b)
    {
        return a.first > b.first;
    };
    std::priority_queue<PathEntry, std::vector<PathEntry>, decltype(cmp)> candidates(cmp);

    for (int i = 1; i < k; ++i)
    {
        const Path &lastPath = result.back();
        for (size_t j = 0; j < lastPath.size() - 1; ++j)
        {
            std::string spurNode = lastPath[j];
            Path rootPath(lastPath.begin(), lastPath.begin() + j + 1);

            std::set<std::pair<std::string, std::string>> removedEdges;
            std::unordered_map<std::pair<std::string, std::string>, Edge, pair_hash> tmpEdges = this->edges;

            for (const auto &p : result)
            {
                if (p.size() > j && std::equal(rootPath.begin(), rootPath.end(), p.begin()))
                {
                    removedEdges.insert({p[j], p[j + 1]});
                }
            }
            for (const auto &e : removedEdges)
                tmpEdges.erase(e);

            auto sub = dijkstra(spurNode, tmpEdges);
            auto spurPath = reconstructPath(spurNode, target, sub.prev);
            if (spurPath.empty())
                continue;

            Path totalPath = rootPath;
            totalPath.insert(totalPath.end(), spurPath.begin() + 1, spurPath.end());
            float totalCost = pathCost(totalPath);
            candidates.push({totalCost, totalPath});
        }

        if (candidates.empty())
            break;
        result.push_back(candidates.top().second);
        candidates.pop();
    }

    return result;
}

bool TaskGenerator::feasible(std::vector<std::string> sources, std::vector<std::string> targets)
{
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, std::string> matchToTarget;
    std::unordered_map<std::string, bool> visited;

    for (const std::string &source : sources)
    {
        auto result = dijkstra(source, this->edges);
        for (const std::string &target : targets)
        {
            if (result.dist.find(target) != result.dist.end() &&
                result.dist[target] != std::numeric_limits<float>::infinity())
            {
                adj[source].push_back(target);
            }
        }
    }

    std::function<bool(const std::string &)> bpm = [&](const std::string &u)
    {
        for (const std::string &v : adj[u])
        {
            if (!visited[v])
            {
                visited[v] = true;
                if (matchToTarget.find(v) == matchToTarget.end() || bpm(matchToTarget[v]))
                {
                    matchToTarget[v] = u;
                    return true;
                }
            }
        }
        return false;
    };

    int matchCount = 0;
    for (const std::string &source : sources)
    {
        visited.clear();
        if (bpm(source))
        {
            matchCount++;
        }
    }

    return matchCount > 0;
}

bool TaskGenerator::canAssign(std::string source, std::string target)
{
    std::unordered_set<std::string> visited;
    std::queue<std::string> q;
    q.push(source);
    visited.insert(source);

    while (!q.empty())
    {
        std::string current = q.front();
        q.pop();

        if (current == target)
            return true;

        auto it = toNodes.find(current);
        if (it != toNodes.end())
        {
            for (const std::string &neighbor : it->second)
            {
                if (visited.count(neighbor) == 0)
                {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
    }
    return false;
}