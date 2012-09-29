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
        pktQueueElementLifetime = par("pktQueueElementLifetime");
        pktQueueCheckingPeriod = par("pktQueueCheckingPeriod");
        debug = par("debug").boolValue();
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";
    	INITIAL_DELAY = 5;
    	BEACON_INTERVAL = 1;
    	selfTimer = new cMessage("beacon-timer",STATIC_NODE_SIG_TIMER);
    	scheduleAt(simTime() + INITIAL_DELAY +uniform(0,0.001), selfTimer);
    }
}

StaticNodeAppLayerHoHuT::~StaticNodeAppLayerHoHuT() {}

void StaticNodeAppLayerHoHuT::finish()
{
    destroyPktQueue();
}

void StaticNodeAppLayerHoHuT::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
        case STATIC_NODE_SIG_TIMER:
            //sendStaticNodeMsg();
            if (this->getParentModule()->getIndex()==0)
            {
                sendStaticNodeMsg("test");
            }

//            if (!selfTimer->isScheduled())
//            {
//                scheduleAt(simTime()+BEACON_INTERVAL,selfTimer);
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
            debugEV << "msg data:" << m->getData() << endl;
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
    ApplPkt* pkt = static_cast<ApplPkt*>(msg);
    switch (msg->getKind())
    {
        case HAS_ROUTE_ACK:
            debugEV << "HAS_ROUTE_ACK received from node: " << pkt->getSrcAddr() << " for destination :" << pkt->getDestAddr();
            break;
        case DELIVERY_ACK:
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
void StaticNodeAppLayerHoHuT::sendStaticNodeMsg(const char* msgData)
{
    LAddress::L3Type destAddr = 1003;

    debugEV << "Sending static node msg" << endl;
    HoHuTApplPkt* aodvTestDataMsg = new HoHuTApplPkt("aodv-test",STATIC_NODE_MSG);
    aodvTestDataMsg->setData(msgData);
    NetwControlInfo::setControlInfo(aodvTestDataMsg, destAddr);

    if (destAddr==LAddress::L3BROADCAST)
    {
        debugEV << "It is broadcast. Send it imediately!" << endl;
        sendDown(aodvTestDataMsg);
    }
    else
    {
        debugEV << "It is not broadcast. Ask netw through the control channel if the route exists for node: " << destAddr << endl;
        addToPktQueue(aodvTestDataMsg);
        HoHuTApplPkt* ctrlPkt = new HoHuTApplPkt("ask-netw-for-route", HAS_ROUTE);
        ctrlPkt->setDestAddr(destAddr);
        sendControlDown(ctrlPkt);
    }
}

/// PACKET QUEUE
void StaticNodeAppLayerHoHuT::destroyPktQueue()
{
    int count = 0;
    while (!pktQueue.empty())
    {
        pktQueueElement *qEl = pktQueue.back();
        pktQueue.pop_back();
        HoHuTApplPkt* pkt = static_cast<HoHuTApplPkt*>(qEl->packet);
        delete pkt;
        count++;
    }
    debugEV << count << " pkts were destroyed" << endl;
}
void StaticNodeAppLayerHoHuT::addToPktQueue(HoHuTApplPkt * pkt)
{
    debugEV << "Adding packet to queue!" << endl;
    pktQueueElement* qEl = (struct pktQueueElement *) malloc(sizeof(struct pktQueueElement));
    if (qEl==NULL)
    {
        error("pktQueueElement malloc failed");
    }
    qEl->destAddr = pkt->getDestAddr();
    qEl->lifeTime = simTime()+pktQueueElementLifetime;
    qEl->packet = pkt;
    pktQueue.push_back(qEl);
}
void StaticNodeAppLayerHoHuT::checkPktQueue()
{
    debugEV << "Checking packet queue for packets to send or expire!" << endl;
}
