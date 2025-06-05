#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include "XMLProcessor.h"
#include "ET.h"
#include <iostream>
XMLProcessor::XMLProcessor(const std::string &filePath)
{
    ET *root = new ET("root");
    root->loadXML(filePath);
    std::unordered_map<std::string, ET *> map_edge;

    std::vector<ET *> edges = root->findAll("edge", {{"id", "^[^:].*"}});
    for (ET *edge : edges)
    {
        std::string id = edge->getData("id");
        map_edge[id] = edge;
        ET* lane = edge->getChildren()[0];
        this->edges[id] = {std::stof(lane->getData("length")), std::stof(lane->getData("speed"))};
    }
    std::vector<ET *> connections = root->findAll("connection", {{"from", "^[^:].*"}, {"to", "^[^:].*"}});
    for (ET *connection : connections)
    {
        std::string fromEdge = connection->getData("from");
        std::string toEdge = connection->getData("to");
        if(map_edge.find(toEdge)==map_edge.end())continue;
        int fromlaneind = std::stoi(connection->getData("fromLane"));
        int tolaneind = std::stoi(connection->getData("toLane"));
        ET* fromLane = map_edge[fromEdge]->getChildren()[fromlaneind];
        ET* toLane = map_edge[toEdge]->getChildren()[tolaneind];
        this->toEdge[fromEdge].push_back(Edge(toEdge, std::stof(toLane->getData("length")), std::stof(toLane->getData("speed"))));
        this->fromEdge[toEdge].push_back(Edge(fromEdge, std::stof(fromLane->getData("length")), std::stof(fromLane->getData("speed"))));
    }
}
