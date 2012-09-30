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
        maxPktQueueElementLifetime = par("maxPktQueueElementLifetime");
        pktQueueMaintenancePeriod = par("pktQueueMaintenancePeriod");
        maxPacketDeliveryTries = par("maxPacketDeliveryTries");
        debug = par("debug").boolValue();
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";
    	INITIAL_DELAY = 5;
    	BEACON_INTERVAL = 1;
    	sentPktQueueTriesCounter = 0;
    	sentPktQueueBuffer = NULL;
    	selfTimer = new cMessage("beacon-timer",STATIC_NODE_SIG_TIMER);
    	scheduleAt(simTime() + INITIAL_DELAY +uniform(0,0.001), selfTimer);
    }
}

StaticNodeAppLayerHoHuT::~StaticNodeAppLayerHoHuT() {}

void StaticNodeAppLayerHoHuT::finish()
{
    destroyPktQueue();
    if (sentPktQueueBuffer!=NULL)
    {
        delete sentPktQueueBuffer;
    }
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
    HoHuTApplPkt* pkt;
    ApplPkt* ctrlPkt = static_cast<ApplPkt*>(msg);

    switch (msg->getKind())
    {
        case HAS_ROUTE_ACK:
            debugEV << "HAS_ROUTE_ACK received for destAddr " << ctrlPkt->getDestAddr() << endl;
            if (sentPktQueueBuffer==NULL)
            {
                pkt = getFromPktQueue();
                sentPktQueueBuffer = pkt->dup();
                NetwControlInfo::setControlInfo(sentPktQueueBuffer, NetwControlInfo::getAddressFromControlInfo(pkt->getControlInfo()));
                if (pkt!=NULL)
                {
                    sentPktQueueTriesCounter = 1;
                    sendDown(pkt);
                }
            }
            else
            {
                sendDown(sentPktQueueBuffer);
            }
            break;
        case DELIVERY_ACK:
            debugEV << "Packet was delivered. SUCCESS!!" << endl;
            sentPktQueueBuffer = NULL;
            sentPktQueueTriesCounter = 0;
            break;
        case DELIVERY_ERROR:
            debugEV << "Packet was not delivered. Try again or giveup" << endl;
            if (sentPktQueueTriesCounter<maxPacketDeliveryTries)
            {
                debugEV << "Number of tries: " << sentPktQueueTriesCounter << " OK. Lets try again!" << endl;
                sentPktQueueTriesCounter++;
                HoHuTApplPkt* ctrlPkt = new HoHuTApplPkt("ask-netw-for-route", HAS_ROUTE);
                ctrlPkt->setDestAddr(NetwControlInfo::getAddressFromControlInfo(sentPktQueueBuffer->getControlInfo()));
                sendControlDown(ctrlPkt);
            }
            else
            {
                debugEV << "Number of tries: " << sentPktQueueTriesCounter << " Give Up!" << endl;
                bubble("PACKET LOST");
                sentPktQueueTriesCounter = 0;
                sentPktQueueBuffer = NULL;
            }
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
void StaticNodeAppLayerHoHuT::sendStaticNodeMsg(const char* msgData)
{
    LAddress::L3Type destAddr = 1003;

    debugEV << "Sending static node msg" << endl;
    HoHuTApplPkt* aodvTestDataMsg = new HoHuTApplPkt("aodv-test",STATIC_NODE_MSG);
    aodvTestDataMsg->setData(msgData);
    NetwControlInfo::setControlInfo(aodvTestDataMsg, destAddr);

    if (destAddr==LAddress::L3BROADCAST)
    {
        debugEV << "It is broadcast. Send it immediately!" << endl;
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
        pktQueueElement* qEl = pktQueue.front();
        pktQueue.pop();
        HoHuTApplPkt* pkt = static_cast<HoHuTApplPkt*>(qEl->packet);
        delete pkt;
        count++;
    }
    debugEV << count << " pkts were destroyed" << endl;
}
HoHuTApplPkt* StaticNodeAppLayerHoHuT::getFromPktQueue()
{
    if (pktQueue.size()>0)
    {
        pktQueueElement* qEl = pktQueue.front();
        pktQueue.pop();
        return qEl->packet;
    }
    return NULL;
}
void StaticNodeAppLayerHoHuT::addToPktQueue(HoHuTApplPkt * pkt)
{
    debugEV << "Adding packet to queue!" << endl;
    pktQueueElement* qEl = (struct pktQueueElement *) malloc(sizeof(struct pktQueueElement));
    if (qEl==NULL)
    {
        error("pktQueueElement malloc failed");
    }
    qEl->destAddr = NetwControlInfo::getAddressFromControlInfo(pkt->getControlInfo());
    qEl->lifeTime = simTime()+maxPktQueueElementLifetime;
    qEl->packet = pkt;
    pktQueue.push(qEl);
}
void StaticNodeAppLayerHoHuT::runPktQueueMaintenance()
{
    debugEV << "Checking packet queue for packets to send or expire!" << endl;
}
