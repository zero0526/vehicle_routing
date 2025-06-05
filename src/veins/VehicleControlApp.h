//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "veins/veins.h"

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "TaskGenerator.h"
using namespace omnetpp;

namespace veins {

/**
 * @brief
 * This is a stub for a typical Veins application layer.
 * Most common functions are overloaded.
 * See MyVeinsApp.cc for hints
 *
 * @author David Eckhoff
 *
 */
struct operationMess{
    std::string senderId;
    std::string action;
    std::string data;
    operationMess(std::string senderId, std::string action, std::string data)
            : senderId(senderId), action(action), data(data) {}
};
class  VehicleControlApp : public TraCIDemo11p {
public:
    void initialize(int stage) override;
    void finish() override;

protected:
    void onBSM(DemoSafetyMessage* bsm) override;
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;
    float calculateDeviation(TimeWindow timeWindow, float departureTime);

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    void handleLowerMsg(cMessage* msg) override;
private:
    simtime_t startTime;
    TimeWindow timewindow;
    float depatureTi;
    bool hasArrive =false;
    bool isMove= false;
    std::string target="";
    std::string originalRoute;
    float absStartTime;
    std::string toJson();
    operationMess fromJson(std::string s);
    bool hasStopped = false;
    int subscribedServiceId = 0;
    cMessage* sendBeacon;
    bool firstMessage = true;
    bool first=true;
};

} // namespace veins
