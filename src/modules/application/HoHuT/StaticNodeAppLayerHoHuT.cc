#include "StaticNodeAppLayerHoHuT.h"

Define_Module(StaticNodeAppLayerHoHuT);

void StaticNodeAppLayerHoHuT::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if (stage == 0) //read config params
    {
    	debugEV<< "in initialize() stage 0...";
        lowerLayerOut = findGate("lowerLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerControlOut = findGate("lowerControlOut");
        lowerControlIn = findGate("lowerControlIn");

        packetMapMaxPktQueueElementLifetime = par("packetMapMaxPktQueueElementLifetime");
        packetMapMaintenancePeriod = par("packetMapMaintenancePeriod");

        nodeSigStartingTime = par("nodeSigStartingTime");
        nodeSigPeriod = par("nodeSigPeriod");

        debug = par("debug").boolValue();
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";
    	selfTimer = new cMessage("beacon-timer",STATIC_NODE_SIG_TIMER);
    	scheduleAt(simTime() + nodeSigStartingTime +uniform(0,0.001), selfTimer);
    }
}

StaticNodeAppLayerHoHuT::~StaticNodeAppLayerHoHuT() {}

void StaticNodeAppLayerHoHuT::finish()
{
    destroyPktMap();
}

void StaticNodeAppLayerHoHuT::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
        case STATIC_NODE_SIG_TIMER:
            //sendStaticNodeMsg();
            if (this->getParentModule()->getIndex()==0)
            {
                char payload[] = "test";
                sendStaticNodeMsg(payload,1003);
            }

//            if (!selfTimer->isScheduled())
//            {
//                scheduleAt(simTime()+nodeSigPeriod,selfTimer);
//            }
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
}

void StaticNodeAppLayerHoHuT::handleLowerMsg(cMessage * msg)
{
    NetwToApplControlInfo* cInfo = static_cast<NetwToApplControlInfo*>(msg->removeControlInfo());
    HoHuTApplPkt* m = static_cast<HoHuTApplPkt*>(msg);

    switch (msg->getKind())
    {
        case STATIC_NODE_MSG:
            debugEV << "received rssi: " << cInfo->getRSSI() << endl;
            debugEV << "Received a node msg from node: " << cInfo->getSrcNetwAddr() << endl;
            debugEV << "msg data:" << m->getPayload() << endl;
            break;
        case STATIC_NODE_SIG:
            debugEV << "Received a node sig from node" << endl;
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
}

void StaticNodeAppLayerHoHuT::handleLowerControl(cMessage* msg)
{
    HoHuTApplPkt* pkt;
    ApplPkt* ctrlPkt = static_cast<ApplPkt*>(msg);

    switch (msg->getKind())
    {
        case HAS_ROUTE_ACK:
            pkt = getFromPktMap(ctrlPkt->getDestAddr());
            NetwControlInfo::setControlInfo(pkt, pkt->getNetwDestAddr());
            sendDown(pkt);
            break;
        case DELIVERY_ACK:
            debugEV << "Packet was delivered. SUCCESS!!" << endl;
            break;
        case DELIVERY_ERROR:
            debugEV << "Packet was not delivered. Packet was lost!" << endl;
            bubble("PACKET LOST");
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
}

/**** APP ****/
void StaticNodeAppLayerHoHuT::sendStaticNodeSig()
{
    debugEV << "Sending SIGNATURE" << endl;
    HoHuTApplPkt* nodeSignature = new HoHuTApplPkt("node-sig",STATIC_NODE_SIG);
    NetwControlInfo::setControlInfo(nodeSignature, nodeSignature->getDestAddr());
    sendDown(nodeSignature);
}
void StaticNodeAppLayerHoHuT::sendStaticNodeMsg(char* msgPayload, LAddress::L3Type netwDestAddr)
{
    debugEV << "Sending static node msg to netwDestAddr:" << netwDestAddr << endl;

    HoHuTApplPkt* appPkt = new HoHuTApplPkt("static-node-app-msg",STATIC_NODE_MSG);
    appPkt->setPayload(msgPayload);
    appPkt->setNetwDestAddr(netwDestAddr);

    if (appPkt->getNetwDestAddr()==LAddress::L3BROADCAST)
    {
        debugEV << "It is broadcast. Send it immediately!" << endl;
        NetwControlInfo::setControlInfo(appPkt, appPkt->getNetwDestAddr());
        sendDown(appPkt);
    }
    else
    {
        debugEV << "It is not broadcast. Ask netw through the control channel if the route exists for node: " << appPkt->getNetwDestAddr() << endl;
        addToPktMap(appPkt);
        ApplPkt* ctrlPkt = new ApplPkt("ask-netw-for-route", HAS_ROUTE);
        ctrlPkt->setDestAddr(appPkt->getNetwDestAddr());
        sendControlDown(ctrlPkt);
    }
}

/// PACKET MAP
void StaticNodeAppLayerHoHuT::destroyPktMap()
{
    int count = 0;
    pktMap_t::iterator it;
    pktQueue_t pktQueue;

    for (it=pktMap.begin(); it!=pktMap.end(); it++)
    {
       while (it->second.size()>0)
       {
           pktQueueElement* qEl = it->second.front();
           it->second.pop();
           HoHuTApplPkt* pkt = static_cast<HoHuTApplPkt*>(qEl->packet);
           delete pkt;
           count++;
       }
       debugEV << count << " pkts were destroyed for destAddr:" << it->first << endl;
    }

}
void StaticNodeAppLayerHoHuT::addToPktMap(HoHuTApplPkt * pkt)
{
    debugEV << "Adding packet to map for address :" << pkt->getNetwDestAddr() << endl;
    pktQueueElement* qEl = (struct pktQueueElement *) malloc(sizeof(struct pktQueueElement));
    qEl->destAddr = pkt->getNetwDestAddr();
    qEl->lifeTime = simTime()+packetMapMaxPktQueueElementLifetime;
    qEl->packet = pkt;

    pktMap_t::iterator it = pktMap.find(pkt->getNetwDestAddr());
    if (it!=pktMap.end())
    {
        it->second.push(qEl);
    }
    else
    {
        pktQueue_t pktQueue;
        pktQueue.push(qEl);
        pktMap.insert(std::pair<LAddress::L3Type,pktQueue_t>(pkt->getNetwDestAddr(),pktQueue));
    }
}
HoHuTApplPkt* StaticNodeAppLayerHoHuT::getFromPktMap(LAddress::L3Type destAddr)
{
    pktMap_t::iterator it = pktMap.find(destAddr);
    if (it!=pktMap.end())
    {
        if (it->second.size()>0)
        {
            pktQueueElement* qEl = it->second.front();
            it->second.pop();
            HoHuTApplPkt* pkt = static_cast<HoHuTApplPkt*>(qEl->packet);
            if (it->second.size()==0)
            {
                pktMap.erase(it);
            }
            return pkt;
        }
    }
    return NULL;
}
void StaticNodeAppLayerHoHuT::runPktMapMaintenance()
{
    debugEV << "Checking packet queue for packets to send or expire!" << endl;
}
