#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include "XMLProcessor.h"
#include "ET.h"
#include<iostream>
XMLProcessor::XMLProcessor(const std::string &filePath)
{
    ET *root = new ET("root");
    root->loadXML(filePath);
    std::vector<ET *> edges = root->findAll("edge", {{"id", "^[^:].*"}, {"from", "^[^J].*"}, {"to", "^[^J].*"}});
    std::unordered_map<std::string, ET *> nodes;
    std::unordered_map<std::string, std::pair<std::string, std::string>> mapEdges;
    for (ET *edge : edges)
    {
        std::string id = edge->getData("id");
        std::string from = edge->getData("from");
        std::string to = edge->getData("to");
        mapEdges[id] = {from, to};
        this->edges[{from, to}] = Edge(id);
        this->nodes.insert(from);
        this->nodes.insert(to);
        nodes[id] = edge;

        this->fromNodes[from].push_back(to);
        this->toNodes[to].push_back(from);
    }
    std::vector<ET *> connections = root->findAll("connection", {{"from", "^[^:].*"}, {"to", "^[^:].*"}});
    for (ET *connection : connections)
    {
        std::string fromEdge = connection->getData("from");
        std::string toEdge = connection->getData("to");
        if(mapEdges.find(fromEdge)==mapEdges.end())continue;
        if(mapEdges.find(toEdge)==mapEdges.end())continue;

        int fromLaneIdx = std::stoi(connection->getData("fromLane"));
        int toLaneIdx = std::stoi(connection->getData("toLane"));
        ET *fromLane = nodes[fromEdge]->getChildren()[fromLaneIdx];
        this->edges[mapEdges[fromEdge]] = Edge(fromEdge, std::stof(fromLane->getData("length")), std::stof(fromLane->getData("speed")));

        ET *toLane = nodes[toEdge]->getChildren()[toLaneIdx];

        this->edges[mapEdges[toEdge]] = Edge(toEdge, std::stof(toLane->getData("length")), std::stof(toLane->getData("speed")));
    }
}