#include "RSUControlApp.h"
#include "Constant.h"
#include "Json.h"
#include <sstream>
#include <cstdlib>
#include<fstream>
#include<iostream>
#include<sstream>
#include<unordered_map>
#include<vector>
#include<list>
#include <random>
#include <ctime>
#include <algorithm>
#include <unordered_set>
#include "ET.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
//#include "DijkstraPath.h"
#include "XMLProcessor.h"
#include "GraphProcessor.h"
#include "TaskGenerator.h"
#include "PairAssigner.h"
#include "Utils.h"
using namespace veins;

Register_Class(RSUControlApp);

void RSUControlApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sendBeacon= new cMessage("send Beacon");

    }
    else if (stage == 1) {
        loadData();
    }


}

void RSUControlApp::finish()
{
    DemoBaseApplLayer::finish();
}

void RSUControlApp::onBSM(DemoSafetyMessage* bsm)
{
//    EV<<"Position: "<<bsm->getSenderPos().str()<<endl;
//    EV<<"Speed: "<<bsm->getSenderSpeed().str()<<bsm->getPsid()<<endl;
}
DemoMessageData RSUControlApp::fromJson(std::string s){
    DemoMessageData d;
    Json mess;
    mess.parseJson(s);
    Json messD;
    messD.parseJson(mess.get("data"));
    d.speed = std::stod(messD.get("speed"));
    d.roadId = messD.get("roadId");
    d.routeId = messD.get("routeId");
    return d;
}
std::string RSUControlApp::toJson(operationMess om){
    Json jMess;
    jMess.add("targetId", om.targetId);
    jMess.add("action", om.action);
    jMess.add("data", om.data);
    return jMess.toJson();
}
void RSUControlApp::loadData(){
    XMLProcessor root("/home/ad/omnetpp-6.1/samples/week6/simulations/veins/map.net.xml");
    this->nodes = root.getNodes();
    this->edges = root.getEdges();
    this->toNodes = root.getToNodes();
    TaskGenerator task(root.getNodes(), root.getEdges(), root.getToNodes());
    std::unordered_map<std::string, std::pair<float, float>>assign =task.genTImeWindow(10);
    for(auto[k, v]:assign){
        EV<<k<<" "<<v.first<<" "<<v.second<<endl;
    }
    int cnt = 0;
    for(std::string n:nodes){
        if(assign.find(n)!=assign.end())continue;
        cnt+=1;
        this->sources.push_back(n);
        if(cnt==10)break;
    }
    std::vector<std::string> targets;
    for(auto[k, v]:assign)targets.push_back(k);
    bool feasible = task.feasible(this->sources, targets);
    if(feasible){
        for(std::string s: this->sources)EV<<s<<" ";
        EV<<endl;
        for(std::string t: targets)EV<<t<<" ";
    }else{
        EV<<"False"<<endl;
    }
    for(auto[k,v]: this->edges){
        this->E2Node[v.id] = {k.first, k.second};
    }
}
//int RSUControlApp::countVehicle(){
//    ET* root = new ET("root");
//    root->loadXML("/home/ad/omnetpp-6.1/samples/week6/simulations/veins/map.rou.xml");
//    vector<ET*> vehicles = root->findAll("vehicle");
//    for(ET* v: vehicles){
//        this->sources.push_back(this->E2node[Utils::S2Edge(v.findNode("route")->getData("edges"))].first);
//    }
//    return vehicles.size();
//}
//std::pair<std::string,float> RSUControlApp::getNewPath(std::string routeId){
//    std::istringstream iss(routeId);
//    std::string word;
//
//    std::string source, target;
//    bool isFirst = true;
//    while (iss >> word) {
//        if (isFirst) {
//            source = word;
//            isFirst = false;
//        }
//        target = word;
//    }
//    std::vector<std::string> path = dp.findShortestPath(source, target);
//    std::string spath="";
//    for (size_t i = 0; i < path.size(); ++i) {
//        spath += path[i];
//        if (i < path.size() - 1) spath += " ";
//    }
//    float cost = dp.CalcuShortestPath(source, target);
//    return {spath, cost};
//}

void RSUControlApp::onWSM(BaseFrame1609_4* wsm)
{
    cPacket* enc = wsm->getEncapsulatedPacket();
    if(TraCIDemo11pMessage* bc = dynamic_cast<TraCIDemo11pMessage*>(enc)){
        std::string message(bc->getDemoData());
        Json jMess;
        EV<<message<<endl;
        jMess.parseJson(message);
        std::string typeMess = jMess.get("type");
        EV<<bc->getSenderAddress()<<" "<<jMess.get("routeId")<<endl;
        if(strcmp(Constant::FIRST,typeMess.c_str()) == 0){

            std::string s =  bc->getDemoData();
            DemoMessageData d = fromJson(s);
            EV<<"aby :"<<d.routeId<<endl;
            d.senderId = bc->getSenderAddress();
            if(sendBeacon != NULL){
                if(sendBeacon->isScheduled()){
                    cancelEvent(sendBeacon);
                }

//            auto [newRoadIds, cost] = getNewPath(d.routeId);
//            if(newRoadIds==""){
//                EV<<-1<<"not found";
//                newRoadIds = d.routeId;
//            }
//            EV<<newRoadIds<< " "<<cost<<" "<<bc->getSenderAddress()<<endl;
            TraCIDemo11pMessage* rsuBeacon = new TraCIDemo11pMessage();
            std::string targetId  = std::string(mergeContent(d.senderId));
            std::string action = "changeRoute";

            Json jData;
            jData.add("roadIds","newroadId");
            operationMess om = operationMess(targetId, action, jData.toJson());
            std::string message = toJson(om);
            rsuBeacon->setDemoData(message.c_str());

            rsuBeacon->setSenderAddress(myId);
            BaseFrame1609_4* WSM = new BaseFrame1609_4();
            WSM->encapsulate(rsuBeacon);
            populateWSM(WSM);
            send(WSM,lowerLayerOut);
            }
        }else{
            // std::string s =  bc->getDemoData();
            // DemoMessageData d = fromJson(s);
            // EV<<"aby :"<<d.routeId<<endl;
            // d.senderId = bc->getSenderAddress();
            // if(sendBeacon != NULL){
            //     if(sendBeacon->isScheduled()){
            //         cancelEvent(sendBeacon);
            //     }

            // auto [newRoadIds, cost] = getNewPath(d.routeId);
            // if(newRoadIds==""){
            //     EV<<-1<<"not found";
            //     newRoadIds = d.routeId;
            // }
            // EV<<newRoadIds<< " "<<bc->getSenderAddress()<<endl;
            // TraCIDemo11pMessage* rsuBeacon = new TraCIDemo11pMessage();
            // std::string targetId  = std::string(mergeContent(d.senderId));
            // std::string action = "kafka";

            // Json jData;
            // jData.add("roadIds",newRoadIds);
            // operationMess om = operationMess(targetId, action, jData.toJson());
            // std::string message = toJson(om);
            // rsuBeacon->setDemoData(message.c_str());

            // rsuBeacon->setSenderAddress(myId);
            // BaseFrame1609_4* WSM = new BaseFrame1609_4();
            // WSM->encapsulate(rsuBeacon);
            // populateWSM(WSM);
            // send(WSM,lowerLayerOut);
//            }
        }
    }
}

void RSUControlApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void RSUControlApp::handleSelfMsg(cMessage* msg)
{
    DemoBaseApplLayer::handleSelfMsg(msg);
}

void RSUControlApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class

}
