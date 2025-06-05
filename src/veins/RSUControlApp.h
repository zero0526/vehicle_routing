//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef VEINS_INET_RSUCONTROLAPP_H_
#define VEINS_INET_RSUCONTROLAPP_H_

#pragma once

#include "veins/veins.h"

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemoRSU11p.h"
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include<unordered_map>
#include <vector>
#include<list>
//#include "DijkstraPath.h"
#include "XMLProcessor.h"
#include "GraphProcessor.h"
#include "TaskGenerator.h"
#include "PairAssigner.h"
#include "HungarianAlgo.h"

using namespace omnetpp;

namespace veins {
struct DemoMessageData{
    double speed;
    std::string roadId;
    std::string laneId;
    std:: string routeId;
    long senderId;
};
struct operationMess{
    std::string targetId;
    std::string action;
    std::string data;

    operationMess(std::string targetId, std::string action, std::string data)
            : targetId(targetId), action(action), data(data) {}
};
struct predict{
    std::string routeId;
    float startTime;
    float depatureTime;
    TimeWindow timeWindow;
    predict(std::string routeId, float startTime, TimeWindow timeWindow, float depatureTime)
            : routeId(routeId), startTime(startTime), timeWindow(timeWindow), depatureTime(depatureTime) {}
    predict(){}
};
class RSUControlApp : public TraCIDemoRSU11p {
public:
    void initialize(int stage) override;
    void finish() override;
    std::string toJson(operationMess om);
    DemoMessageData fromJson(std::string bc);
    std::unordered_map<std::string, std::vector<Edge>> fromEdges;
    std::unordered_map<std::string, std::vector<Edge>> toEdges;
    std::unordered_map<std::string, std::pair<float, float>> edges;
    std::unordered_map<std::string, std::pair<std::string, float>> sourceCar;
    std::unordered_map<std::string, predict> resultsRoute;
    void loadData();
    void initCSV(const std::string& filename);
    predict getNewRoute(std::string oriRoute);

protected:
    void onBSM(DemoSafetyMessage* bsm) override;
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
private:
    int totalCar;
    double timeExecute;
    bool hasStopped = false;
    int subscribedServiceId = 0;
    cMessage* sendBeacon;
};
}

#endif /* VEINS_INET_RSUCONTROLAPP_H_ */

