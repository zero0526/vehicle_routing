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
#include "XMLProcessor.h"
#include "GraphProcessor.h"
#include "TaskGenerator.h"
#include "PairAssigner.h"
#include "HungarianAlgo.h"
#include <cctype>
#include <cstdlib>

using namespace veins;

Register_Class(RSUControlApp);

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    size_t end = s.find_last_not_of(" \t\n\r\f\v");

    if (start == std::string::npos)
        return "";

    return s.substr(start, end - start + 1);
}

void RSUControlApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sendBeacon= new cMessage("send Beacon");

    }
    else if (stage == 1) {
        loadData();
        initCSV("vehicle.csv");
    }
}
void RSUControlApp::initCSV(const std::string& filename){
    std::ofstream file;
    file.open("vehicle.csv", std::ios::app);
    if (file.is_open()) {
        file<<"Xe_ID,OriginalRoute,Thoi_gian_chay,Do_lech,Gia_tri_ham_muc_tieu"<<"\n";
        file.close();
    }
    else {
        EV << "Khong the mo file: " << "vehicle.csv" << endl;
    }
}

void RSUControlApp::finish()
{
    std::ofstream file;
    file.open("vehicle.csv", std::ios::app);
    if (file.is_open()) {
        file<<"totalcar,timeExecute"<<"\n";
        file <<totalCar<<","<< timeExecute<< "\n";
        file.close();
        system("python3 /home/ad/Documents/PracticeCpp/log2ggdriver.py");
    }
    else {
        EV << "Khong the mo file: " << "vehicle.csv" << endl;
    }
    DemoBaseApplLayer::finish();

}

void RSUControlApp::onBSM(DemoSafetyMessage* bsm)
{
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
std::string toString(std::vector<std::string> vec){
    std::string str="";
    for(std::string el:vec){
        if(!str.length())str+=el;
        else str+= " " + el;
    }
    return str;
}
void RSUControlApp::loadData(){
    XMLProcessor root("/home/ad/omnetpp-6.1/samples/week6/simulations/veins/map.net.xml");
    toEdges = root.getToEdge();
    fromEdges = root.getFromEdge();
    edges = root.getEdges();
    ET* roota = new ET("root");
    roota->loadXML("/home/ad/omnetpp-6.1/samples/week6/simulations/veins/map.rou.xml");
    std::vector<ET*> vehicles = roota->findAll("vehicle");
    totalCar = vehicles.size();
    for (ET* vehicle : vehicles) {
        std::string route = vehicle->findNode("route")->getData("edges");
        float startTime = std::stof(vehicle->getData("depart"));
        std::istringstream iss(route);
        std::string sourceC;
        iss>> sourceC;
        sourceCar[sourceC] = {route, startTime};
    }
    GraphProcessor graphProcessor(fromEdges, toEdges);
    TaskGenerator taskGenerator(fromEdges, toEdges, edges);
    std::vector<std::pair<std::string, float>> sources;

    for (const auto& source : sourceCar) {
        sources.push_back({source.first,source.second.second});
    }
    std::unordered_map<std::string, TimeWindow> timewindows = taskGenerator.genTimeWindow(sources.size());
    PairAssigner pairAssigner(sources, timewindows, edges, taskGenerator);
    int i = 0;
    simtime_t getStarted = simTime();
    std::vector<AssignmentResult> results = pairAssigner.assign();
    timeExecute = (simTime() - getStarted).dbl()*1000.0;
    for(AssignmentResult rs: results){
        std::string originalRoute= sourceCar[rs.source].first;
        resultsRoute[originalRoute] = {toString(rs.path), sourceCar[rs.source].second, rs.timeWindow, rs.departureTime};
    }
}

predict RSUControlApp::getNewRoute(std::string oriRoute){
    return resultsRoute[trim(oriRoute)];
}
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
            d.senderId = bc->getSenderAddress();

            if(sendBeacon != NULL){
                if(sendBeacon->isScheduled()){
                    cancelEvent(sendBeacon);
                }

            TraCIDemo11pMessage* rsuBeacon = new TraCIDemo11pMessage();
            std::string targetId  = std::string(mergeContent(d.senderId));
            std::string action = "changeRoute";

            Json jData;
            predict predictData = getNewRoute(d.routeId);
            std::string newRouteId = predictData.routeId;
            float stime = predictData.startTime;
            TimeWindow timeWindow = predictData.timeWindow;
            float departTime = predictData.depatureTime;
            jData.add("roadIds",newRouteId);
            jData.add("starttime", std::to_string(stime));
            jData.add("early", std::to_string(timeWindow.earlyTime));
            jData.add("lately", std::to_string(timeWindow.lateTime));
            jData.add("departureTime",std::to_string(departTime));
            operationMess om = operationMess(targetId, action, jData.toJson());
            std::string message = toJson(om);
            rsuBeacon->setDemoData(message.c_str());

            rsuBeacon->setSenderAddress(myId);
            BaseFrame1609_4* WSM = new BaseFrame1609_4();
            WSM->encapsulate(rsuBeacon);
            populateWSM(WSM);
            send(WSM,lowerLayerOut);
            }
            else{
                EV<<"sendBeacon is NULL"<<endl;
            }
        }
    }
}

void RSUControlApp::onWSA(DemoServiceAdvertisment* wsa)
{
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
