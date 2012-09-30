#include "AODVRoute.h"

using std::endl;

Define_Module(AODVRoute);

void AODVRoute::initialize(int stage)
{
	BaseNetwLayer::initialize(stage);
	if (stage == 0)
	{
		trace = par("trace");
		debug = par("debug").boolValue();
		maxRREQVectorSize = par("maxRREQVectorSize");
		maxRREQVectorElementLifetime = par("maxRREQVectorElementLifetime");
	}
	else if (stage == 1)
	{
	    RREQSent = 0;
	    nodeSeqNo = 1;
	    debugEV << "Host index=" << findHost()->getIndex() << ", Id=" << findHost()->getId() << endl;
	    debugEV << "  host IP address=" << myNetwAddr << endl;
	    debugEV << "  host macaddress=" << arp->getMacAddr(myNetwAddr) << endl;
	}
}

void AODVRoute::finish(){}

/// MSG HANDLING
void AODVRoute::handleSelfMsg(cMessage * msg)
{
   switch (msg->getKind())
   {
       default:
           error("Unknown message of kind: "+msg->getKind());
           break;
   }
   delete msg;
}
//FROM APPL
void AODVRoute::handleUpperMsg(cMessage * msg)
{
    ApplPkt* appPkt = static_cast<ApplPkt*>(msg);
    LAddress::L3Type destAddress = NetwControlInfo::getAddressFromControlInfo(appPkt->getControlInfo());

    if (destAddress==LAddress::L3BROADCAST || hasRouteForDestination(destAddress))
    {
        AODVData* m = encapsMsg(appPkt);
        debugEV << "using route and sending down to destAddr:" << destAddress << endl;
        sendDown(m);
    }
    else
    {
        error("Route was not found, but it should exist!");
        delete msg;
    }
}
void AODVRoute::handleUpperControl(cMessage* msg)
{
    ApplPkt* ctrlPkt = static_cast<ApplPkt*>(msg);
    switch (msg->getKind())
    {
        case HAS_ROUTE:
            handleUpperControlHasRoute(ctrlPkt);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
}
//FROM MAC
void AODVRoute::handleLowerMsg(cMessage* msg)
{
    switch (msg->getKind())
    {
        case DATA:
            {
                AODVData* m = static_cast<AODVData*>(msg);
                ApplPkt* pkt = static_cast<ApplPkt*>(decapsMsg(m));
                sendUp(pkt);
            }
            break;
        case RREQ:
            handleLowerRREQ(msg);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void AODVRoute::handleLowerControl(cMessage* msg)
{
    ApplPkt* pkt;
    switch(msg->getKind())
    {
        case PACKET_DROPPED:
            debugEV << "MAC ERROR detected! Inform app layer!" << endl;
            pkt = new ApplPkt("last-transmitted-packet-dropped",DELIVERY_ERROR);
            sendControlUp(pkt);
            break;
        case TX_OVER:
            debugEV << "Transmission success!" << endl;
            pkt = new ApplPkt("last-transmitted-packet-success",DELIVERY_ACK);
            sendControlUp(pkt);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
    delete msg;
}



/// HANDLE COMPLEX MSG KINDS
void AODVRoute::handleLowerRREQ(cMessage* msg)
{
    AODVRouteRequest* pkt = static_cast<AODVRouteRequest*>(msg);
    if (!hasRREQ(pkt))
    {
        //intermediate node
        if (pkt->getInitialSrcAddr()!=myNetwAddr && pkt->getFinalDestAddr()!=myNetwAddr)
        {
            debugEV << "INTERMIDIATE NODE - received a RREQ from node:" << pkt->getSrcAddr() << endl;
            saveRREQ(pkt);
            pkt->removeControlInfo();
            NetwToMacControlInfo::setControlInfo(pkt,LAddress::L2BROADCAST);
            sendDown(pkt);
        }
        //destination
        else if (pkt->getFinalDestAddr()==myNetwAddr)
        {
            delete msg;
        }
        //source
        else
        {
            delete msg;
        }
    }
    else
    {
        delete msg;
    }
}
void AODVRoute::handleUpperControlHasRoute(ApplPkt* ctrlPkt)
{
    debugEV << "HAS_ROUTE(?) received for destAddr: " << ctrlPkt->getDestAddr() << endl;
    if (hasRouteForDestination(ctrlPkt->getDestAddr()))
    {
        debugEV << "Route found for destAddr: " << ctrlPkt->getDestAddr() << endl;
        ApplPkt* ctrlPktResponse = new ApplPkt("has-route-ACK",HAS_ROUTE_ACK);
        ctrlPktResponse->setDestAddr(ctrlPkt->getDestAddr());
        sendControlUp(ctrlPktResponse);
    }
    else
    {
        debugEV << "Route not found for destAddr: " << ctrlPkt->getDestAddr() << endl;
        debugEV << "Sending RREQ" << endl;
        RREQSent++;

        AODVRouteRequest* pkt = new AODVRouteRequest("RREQ",RREQ);
        pkt->setInitialSrcAddr(myNetwAddr);
        pkt->setFinalDestAddr(ctrlPkt->getDestAddr());
        pkt->setSrcAddr(myNetwAddr);
        pkt->setDestAddr(LAddress::L3BROADCAST);
        pkt->setRREQ_ID(RREQSent);
        if (!getNodeSeqNo(pkt->getFinalDestAddr()))
        {
            upsertNodeSeqNo(pkt->getFinalDestAddr(),0); //unknown seqno at destinatino node
        }
        pkt->setDestSeqNo(getNodeSeqNo(pkt->getFinalDestAddr()));
        pkt->setSrcSeqNo(nodeSeqNo);
        pkt->setHopCount(0);

        NetwToMacControlInfo::setControlInfo(pkt,LAddress::L2BROADCAST);
        sendDown(pkt);
    }
}


/// OTHER NODES last know seqNo
int AODVRoute::getNodeSeqNo(LAddress::L3Type addr)
{
    std::map<LAddress::L3Type,int>::iterator it = nodesLastKnownSeqNoTable.find(addr);
    if (it!=nodesLastKnownSeqNoTable.end())
    {
        return it->second;
    }
    else
    {
        return false;
    }
}
void AODVRoute::upsertNodeSeqNo(LAddress::L3Type addr,int seqNumber)
{
    std::map<LAddress::L3Type,int>::iterator it = nodesLastKnownSeqNoTable.find(addr);
    if (it==nodesLastKnownSeqNoTable.end())
    {
        nodesLastKnownSeqNoTable[addr] = seqNumber;
    }
    else
    {
        nodesLastKnownSeqNoTable[it->first] = seqNumber;
    }
}


/// ROUTE TABLE
bool AODVRoute::hasRouteForDestination(LAddress::L3Type destAddr)
{
    return false;
}
void AODVRoute::routeTableMaintenance()
{

}
void AODVRoute::getRouteForDestination(LAddress::L3Type)
{

}
void AODVRoute::insertReverseRoute(AODVRouteRequest* msg)
{

}
void AODVRoute::insertForwardRoute(AODVRouteResponse* msg)
{

}


/// ROUTE REQUEST SET
bool AODVRoute::hasRREQ(AODVRouteRequest* rreq)
{
    RREQVectorElement* tEl = (struct RREQVectorElement*) malloc(sizeof(struct RREQVectorElement));
    tEl->srcAddr = rreq->getInitialSrcAddr();
    tEl->RREQ_ID = rreq->getRREQ_ID();

    for (unsigned int i=0; i<RREQVector.size(); i++)
    {
        RREQVectorElement* tElAux = static_cast<RREQVectorElement*>(RREQVector[i]);
        if (tElAux->RREQ_ID==tEl->RREQ_ID && tElAux->srcAddr==tEl->srcAddr)
        {
            return true;
        }
    }
    return false;
}
void AODVRoute::saveRREQ(AODVRouteRequest* rreq)
{
    RREQVectorElement* tEl = (struct RREQVectorElement*) malloc(sizeof(struct RREQVectorElement));
    tEl->srcAddr = rreq->getInitialSrcAddr();
    tEl->RREQ_ID = rreq->getRREQ_ID();
    tEl->lifeTime = simTime()+maxRREQVectorElementLifetime;
    while (RREQVector.size()>=maxRREQVectorSize)
    {
        RREQVector.erase(RREQVector.begin());
    }
    RREQVector.push_back(tEl);
}
void AODVRoute::runRREQSetMaintenance()
{
    //clear all entries that have expired
}


/// ENCAPSULATION
AODVData* AODVRoute::encapsMsg(cPacket* cPkt)
{
    ApplPkt* appPkt = static_cast<ApplPkt*>(cPkt);
    AODVData* pkt = new AODVData(appPkt->getName(), DATA);
    cObject* cInfo = appPkt->removeControlInfo();

    LAddress::L3Type netwDestAddr = NetwControlInfo::getAddressFromControlInfo(cInfo);
    LAddress::L2Type macDestAddr = (LAddress::isL3Broadcast(netwDestAddr))?(LAddress::L2BROADCAST):(arp->getMacAddr(netwDestAddr));

    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setFinalDestAddr(netwDestAddr);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwDestAddr);
    pkt->setBitLength(headerLength);
    pkt->encapsulate(appPkt);
    NetwToMacControlInfo::setControlInfo(pkt, macDestAddr);

    return pkt;
}
cPacket* AODVRoute::decapsMsg(AODVData *msg)
{
    ApplPkt* pkt = static_cast<ApplPkt*>(msg->decapsulate());
    MacToNetwControlInfo* cInfo = static_cast<MacToNetwControlInfo*>(msg->removeControlInfo());
    NetwToApplControlInfo::setControlInfo(pkt,cInfo->getRSSI(), msg->getSrcAddr());

    delete msg;
    return pkt;
}
