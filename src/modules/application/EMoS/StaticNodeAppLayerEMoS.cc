#include "StaticNodeAppLayerEMoS.h"

Define_Module(StaticNodeAppLayerEMoS);

void StaticNodeAppLayerEMoS::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if (stage == 0) //read config params
    {
    	debugEV<< "in initialize() stage 0...";
        lowerLayerOut = findGate("lowerLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerControlOut = findGate("lowerControlOut");
        lowerControlIn = findGate("lowerControlIn");

        nodeSigStartingTime = par("nodeSigStartingTime");
        nodeSigPeriod = par("nodeSigPeriod");
        beaconMode = par("beaconMode");
        baseStationNetwAddr = par("baseStationNetwAddr");

        debug = par("debug").boolValue();
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";

    	if (beaconMode)
    	{
            selfTimer = new cMessage("beacon-timer",STATIC_NODE_SIG_TIMER);
            scheduleAt(simTime() + nodeSigStartingTime +uniform(0,0.001), selfTimer);
    	}
    	else
    	{
            selfTimer = new cMessage("msg-timer",STATIC_NODE_MSG_TIMER);
            scheduleAt(simTime()+uniform(0,0.001),selfTimer);
    	}
    }
}

StaticNodeAppLayerEMoS::~StaticNodeAppLayerEMoS() {}

void StaticNodeAppLayerEMoS::finish()
{
    cancelAndDelete(selfTimer);
}

void StaticNodeAppLayerEMoS::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
        case STATIC_NODE_SIG_TIMER:
            sendStaticNodeSig();
            break;
        case STATIC_NODE_MSG_TIMER:
            //AODV TEST
            if (findHost()->getIndex()==0)
            {
                sendStaticNodeMsg((char*)"test-msg",1003);
            }
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}

void StaticNodeAppLayerEMoS::handleLowerMsg(cMessage * msg)
{
    NetwToApplControlInfo* cInfo;
    EMoSApplPkt* applPkt;

    switch (msg->getKind())
    {
        case STATIC_NODE_MSG:
            cInfo = static_cast<NetwToApplControlInfo*>(msg->removeControlInfo());
            applPkt = static_cast<EMoSApplPkt*>(msg);
            debugEV << "received rssi: " << cInfo->getRSSI() << endl;
            debugEV << "Received a node msg from static node: " << cInfo->getSrcNetwAddr() << endl;
            debugEV << "msg data:" << applPkt->getPayload() << endl;
            delete msg;
            break;
        case MOBILE_NODE_MSG:
            handleMobileNodeMsg(msg);
            break;
        case STATIC_NODE_SIG: //static nodes ignore this msg types. they are only for the mobile nodes
            delete msg;
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void StaticNodeAppLayerEMoS::handleLowerControl(cMessage* msg)
{
    delete msg; //dont do nothing for now
}
void StaticNodeAppLayerEMoS::handleMobileNodeMsg(cMessage* msg)
{
    EMoSApplPkt* applPkt = static_cast<EMoSApplPkt*>(msg);
    NetwToApplControlInfo* cInfo = static_cast<NetwToApplControlInfo*>(msg->removeControlInfo());
    debugEV << "received rssi: " << cInfo->getRSSI() << endl;
    debugEV << "Received a node msg from mobile node: " << cInfo->getSrcNetwAddr() << endl;
    debugEV << "msg data:" << applPkt->getPayload() << endl;

    switch (applPkt->getEMoSMsgType())
    {
        case COLLECTED_RSSI:
            NetwControlInfo::setControlInfo(applPkt,baseStationNetwAddr); //fwd msg to the base station
            sendDown(applPkt);
            break;
        default:
            error("Unknown EMoSMsgType");
            delete msg;
            break;
    }
}
void StaticNodeAppLayerEMoS::sendStaticNodeMsg(char* msgPayload, LAddress::L3Type netwDestAddr)
{
    debugEV << "Sending static node msg to netwDestAddr:" << netwDestAddr << endl;

    EMoSApplPkt* appPkt = new EMoSApplPkt("static-node-app-msg",STATIC_NODE_MSG);
    appPkt->setPayload(msgPayload);
    NetwControlInfo::setControlInfo(appPkt, netwDestAddr);
    sendDown(appPkt);
}
void StaticNodeAppLayerEMoS::sendStaticNodeSig()
{
    debugEV << "Sending SIGNATURE" << endl;
    EMoSApplPkt* appPkt = new EMoSApplPkt("node-sig",STATIC_NODE_SIG);
    NetwControlInfo::setControlInfo(appPkt, LAddress::L3BROADCAST);
    sendDown(appPkt);
    if (!selfTimer->isScheduled())
    {
        scheduleAt(simTime()+nodeSigPeriod,selfTimer);
    }
}
