#pragma once
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "XMLProcessor.h"
#include <limits>
#include "GraphProcessor.h"

struct TimeWindow
{
    float earlyTime;
    float lateTime;
    TimeWindow(){}
    TimeWindow(float earlyTime, float lateTime):earlyTime(earlyTime),lateTime(lateTime) {}
};


class TaskGenerator
{

public:
    TaskGenerator(
        const std::unordered_map<std::string, std::vector<Edge>> fromEdges,
    const std::unordered_map<std::string, std::vector<Edge>> toEdges,
    std::unordered_map<std::string, std::pair<float, float>> edges) : edges(edges), fromEdges(fromEdges), toEdges(toEdges){
        graphProcessor = GraphProcessor(fromEdges, toEdges);
    }
    TaskGenerator():        fromEdges({}),
        toEdges({}),
        graphProcessor() {}
    std::unordered_map<std::string, TimeWindow> genTimeWindow(int n);
    std::pair<std::vector<std::string>, float> findBestTimeWindowPath(std::string source, std::pair<std::string,TimeWindow> target);
    std::string joinPath(const std::vector<std::string>& path);
    float timeFunc(std::vector<std::string>path);
    std::unordered_map<float, std::vector<std::string>> findKShortestPaths(const std::string &sourceId, const std::string &targetId, int k);
    bool feasible(std::vector<std::string> sources, std::vector<std::string> targets);
private:
    std::unordered_map<std::string, std::pair<float, float>> edges;
     std::unordered_map<std::string, std::vector<Edge>> fromEdges;
     std::unordered_map<std::string, std::vector<Edge>> toEdges;
    GraphProcessor graphProcessor;
};
