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

#include "VehicleControlApp.h"
#include "Constant.h"
#include "Json.h"
#include<list>
#include<sstream>
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "TaskGenerator.h"

using namespace veins;

Register_Class(VehicleControlApp);

void VehicleControlApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {

        int idDebug = getId();
        sendBeacon= new cMessage("send Beacon");
        if(idDebug == 15){
            mobility = TraCIMobilityAccess().get(getParentModule());
            traciVehicle = mobility->getVehicleCommandInterface();
            ///subscribedServiceId = -1;
            //currentOfferedServiceId = 7;

            //wsaInterval = 5;
            //beaconInterval = 1;
        }
    }
    else if (stage == 1) {

        if (sendBeacon->isScheduled())
        {
            cancelEvent(sendBeacon);
        }
        scheduleAt(simTime() + 0.1, sendBeacon);
        EV<<"Send to RSU. Waiting for response from RSU!"<<endl;
    }
}
float VehicleControlApp::calculateDeviation(TimeWindow timeWindow, float departureTime){
    if (departureTime < timeWindow.earlyTime)
            return timeWindow.earlyTime - departureTime;
        if (departureTime > timeWindow.lateTime)
            return departureTime - timeWindow.lateTime;
        return 0.0f;
}

void VehicleControlApp::finish()
{
    DemoBaseApplLayer::finish();
    //EV<<"Reach destination over here"<<endl;
    // statistics recording goes here
    simtime_t endTime = simTime();
    double runTime = (endTime - startTime).dbl() + absStartTime;
    std::string atde;
    if(hasArrive){
        atde = "true";
    }else atde="false";
    std::ofstream file;
    file.open("vehicle.csv", std::ios::app);
    if (file.is_open()) {
        file << myId<<","<<atde<<","<<runTime<<","<<calculateDeviation(timewindow, runTime)<<","<<depatureTi<< "\n";
        file.close();
    }
    else {
        EV << "Khong the mo file: " << "vehicle.csv" << endl;
    }
}

std::string VehicleControlApp::toJson(){
    Json message;
    if (firstMessage){
        std::string s(Constant::FIRST);
        message.add("type",s);
    }else{
        message.add("type","casual");
    }
    std::string roadIds;
    for(const std::string& road:traciVehicle->getPlannedRoadIds()){
        roadIds = roadIds + " " + road;
    }
    Json data;
    data.add("speed",std::to_string(traciVehicle->getSpeed()));
    data.add("roadId",traciVehicle->getRoadId());
    data.add("routeId",roadIds);
    message.add("data",data.toJson());
    return message.toJson();
}

void VehicleControlApp::onBSM(DemoSafetyMessage* bsm)
{
    //for my own simulation circle
}

void VehicleControlApp::onWSM(BaseFrame1609_4* wsm)
{
    // Your application has received a data message from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void VehicleControlApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void VehicleControlApp::handleSelfMsg(cMessage* msg)
{
    DemoBaseApplLayer::handleSelfMsg(msg);
    // this method is for self messages (mostly timers)
    // it is important to call the DemoBaseApplLayer function for BSM and WSM transmission
    //if(msg == sendBeacon)
    {
        TraCIDemo11pMessage* carBeacon = new TraCIDemo11pMessage();
        std::string mess = toJson();
        carBeacon->setDemoData(mess.c_str());
        if(firstMessage)
            firstMessage = false;

        carBeacon->setSenderAddress(myId);
        BaseFrame1609_4* WSM = new BaseFrame1609_4();
        WSM->encapsulate(carBeacon);
        populateWSM(WSM);
        send(WSM,lowerLayerOut);
        return;
    }
}

void VehicleControlApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    if(!isMove&&traciVehicle->getSpeed()>0){
        isMove = true;
        startTime = simTime();
    }
    if(traciVehicle->getRoadId()==target){
        hasArrive = true;
    }
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

}

void VehicleControlApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* WSM = check_and_cast<BaseFrame1609_4*>(msg);
    cPacket* enc = WSM->getEncapsulatedPacket();
    if(TraCIDemo11pMessage* bc = dynamic_cast<TraCIDemo11pMessage*>(enc)){
        char *ret = mergeContent(myId);
        std::string message(bc->getDemoData());
        Json jMess;
        jMess.parseJson(message);
        std::string action = jMess.get("action");
        std::cout<<jMess.get("data")<<std::endl;
        Json jdata;

        if(strcmp(ret, jMess.get("targetId").c_str()) == 0){
            if(first){
                first =false;
                 EV<<action<<endl;
                if(strcmp(action.c_str(),"changeRoute") == 0){
                    EV<<"get in sucesss"<<endl;
                    std::string mData = jMess.get("data");
                    EV<<myId<<mData<<" the first"<<endl;
                    jdata.parseJson(mData);
                    std::string roadIds = jdata.get("roadIds");
                    absStartTime = std::stof(jdata.get("starttime"));
                    timewindow = {std::stof(jdata.get("early")), std::stof(jdata.get("lately"))};
                    depatureTi = std::stof(jdata.get("departureTime"));
                    std::stringstream ss(roadIds);
                    std::list<std::string> route;
                    std::string id;
                    while (ss >> id) {
                        route.push_back(id);
                    }
                    EV<<"end: "<<route.back()<<endl;
                    target = route.back();
                    bool insp = traciVehicle->changeVehicleRoute(route);
                    if(insp)
                        EV<<"successfully change route"<<endl;
                    else EV<<"failed !!!!"<<endl;
                 }

            }
        }else{
//            EV<<"Wrong address!! "<<message<<endl;
        }
    }
    else{
        //EV<<"In "<<myId<<". At "<<simTime()
        //        <<" no TraCIDemo11Message but from "
        //        <<msg->getSenderModuleId()<<endl;
    }
}
