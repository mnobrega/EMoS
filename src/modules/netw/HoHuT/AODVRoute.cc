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
		RREQVectorMaxSize = par("RREQVectorMaxSize");
		RREQVectorElementMaxLifetime = par("RREQVectorElementMaxLifetime");
		routeMapElementMaxLifetime = par("routeMapElementMaxLifetime");
		RREQVectorMaintenancePeriod = par("RREQVectorMaintenancePeriod");
		localRepair = par("localRepair").boolValue();
	}
	else if (stage == 1)
	{
	    RREQIDCounter = 0;
	    nodeSeqNo = 0;
	    debugEV << "Host index=" << findHost()->getIndex() << ", Id=" << findHost()->getId() << endl;
	    debugEV << "  host IP address=" << myNetwAddr << endl;
	    debugEV << "  host macaddress=" << arp->getMacAddr(myNetwAddr) << endl;
	}
}

void AODVRoute::finish()
{
    nodesLastKnownSeqNoMap.clear();
    routeMap.clear();
    RREQVector.clear();
}

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
        routeMapElement* fwdRoute = getRouteForDestination(destAddress);
        m->setFinalDestAddr(destAddress);
        m->setInitialSrcAddr(myNetwAddr);
        m->setSrcAddr(myNetwAddr);
        m->setDestAddr(fwdRoute->nextHop);
        m->removeControlInfo();

        //TEST
        //NetwToMacControlInfo::setControlInfo(m,arp->getMacAddr(fwdRoute->nextHop);
        NetwToMacControlInfo::setControlInfo(m,9999); //doesnt exist and will not be delivered
        //TEST

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
            handleLowerDATA(msg);
            break;
        case RREQ:
            handleLowerRREQ(msg);
            break;
        case RREP:
            handleLowerRREP(msg);
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
            {
                debugEV << "MAC ERROR detected! Inform app layer!" << endl;
                MacPkt* macPkt = static_cast<MacPkt*>(msg);
                NetwPkt* netwPkt = static_cast<NetwPkt*>(macPkt->decapsulate());
                if (!localRepair)
                {
                    AODVRouteError* rerr = new AODVRouteError("route-error",RERR);
                    unreachAddSeqNoMap_t addrMap = removeRoutesByNextHop(netwPkt->getDestAddr());
                    rerr->setUnreachDestAddr(netwPkt->getDestAddr());
                }
                else
                {
                    //TODO
                }
                pkt = new ApplPkt("last-transmitted-packet-dropped",DELIVERY_ERROR);
                sendControlUp(pkt);
            }
            break;
        case TX_OVER:
            debugEV << "Transmission success!" << endl;
            pkt = new ApplPkt("last-transmitted-packet-success",DELIVERY_ACK);
            sendControlUp(pkt);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
}



/// HANDLE COMPLEX MSG KINDS
void AODVRoute::handleLowerDATA(cMessage* msg)
{
    AODVData* m = static_cast<AODVData*>(msg);

    if (m->getFinalDestAddr()!=myNetwAddr && m->getInitialSrcAddr()!=myNetwAddr && !LAddress::isL3Broadcast(m->getDestAddr())) //intermediate node and NOT BROADCAST
    {
        debugEV << "DATA msg arrived to intermediate node" << endl;
        routeMapElement* fwdRoute = getRouteForDestination(m->getFinalDestAddr());
        m->setDestAddr(fwdRoute->nextHop);
        m->setSrcAddr(myNetwAddr);
        m->removeControlInfo();
        NetwToMacControlInfo::setControlInfo(m,arp->getMacAddr(fwdRoute->nextHop));
        sendDown(m);
    }
    else if (m->getFinalDestAddr()==myNetwAddr || LAddress::isL3Broadcast(m->getDestAddr())) //DESTINATION
    {
        debugEV << "DATA msg arrived to final destination!" << endl;
        ApplPkt* pkt = static_cast<ApplPkt*>(decapsMsg(m));
        sendUp(pkt);
    }
    else //SOURCE
    {
        delete msg;
    }
}
void AODVRoute::handleLowerRREP(cMessage* msg)
{
    AODVRouteResponse* rrep = static_cast<AODVRouteResponse*>(msg);
    int hopCountToRouteDestination = rrep->getHopCount()+1;
    rrep->setHopCount(hopCountToRouteDestination);
    upsertNodeSeqNo(rrep->getRouteDestAddr(), rrep->getRouteDestSeqNo());

    //intermediate node
    if (rrep->getRouteDestAddr()!=myNetwAddr && rrep->getRouteSrcAddr()!=myNetwAddr)
    {
        debugEV << "Arrived to INTERMEDIATE NODE! Save fwd route and rev msg";
        upsertForwardRoute(rrep);
        routeMapElement* revRoute = getRouteForDestination(rrep->getRouteSrcAddr());
        rrep->setSrcAddr(myNetwAddr);
        rrep->setDestAddr(revRoute->nextHop);
        rrep->removeControlInfo();
        NetwToMacControlInfo::setControlInfo(rrep,arp->getMacAddr(revRoute->nextHop));
        sendDown(rrep);
    }
    //route source node
    else if (rrep->getRouteSrcAddr()==myNetwAddr)
    {
        debugEV << "Arrived to ROUTE SOURCE! Completed ROUTE discovery!!!" << endl;
        upsertForwardRoute(rrep);
        ApplPkt* ctrlPkt = new ApplPkt("route-found",HAS_ROUTE_ACK);
        ctrlPkt->setDestAddr(rrep->getRouteDestAddr());
        sendControlUp(ctrlPkt);
        delete msg;
    }
    //route destination node - ignore
    else if (rrep->getRouteDestAddr()==myNetwAddr)
    {
        debugEV << "Arrived to ROUTE DESTINATION! Ignore the msg!";
        delete msg;
    }
}
void AODVRoute::handleLowerRREQ(cMessage* msg)
{
    AODVRouteRequest* rreq = static_cast<AODVRouteRequest*>(msg);
    if (!hasRREQ(rreq)) //never received this one
    {
        int hopCount = rreq->getHopCount() + 1;
        rreq->setHopCount(hopCount);

        //update the src addr seq number
        upsertNodeSeqNo(rreq->getInitialSrcAddr(),rreq->getInitialSrcSeqNo());

        //intermediate node
        if (rreq->getInitialSrcAddr()!=myNetwAddr && rreq->getFinalDestAddr()!=myNetwAddr)
        {
            debugEV << "INTERMEDIATE NODE - received a RREQ from node:" << rreq->getInitialSrcAddr() << endl;

            saveRREQ(rreq);
            upsertReverseRoute(rreq);
            routeMapElement* fwdRoute = getRouteForDestination(rreq->getFinalDestAddr());

            if (fwdRoute==NULL || (fwdRoute!=NULL && fwdRoute->destSeqNo < rreq->getFinalDestSeqNo()) )
            {
                debugEV << "No valid fwdRoute found! CONTINUE with Route discovery. Broadcast RREQ!" << endl;
                rreq->setSrcAddr(myNetwAddr);
                rreq->removeControlInfo();
                NetwToMacControlInfo::setControlInfo(rreq,LAddress::L2BROADCAST);
                sendDown(rreq);
            }
            else
            {
                debugEV << "FwdRoute found! Unicast RREP in the reverse order" << endl;

                AODVRouteResponse* rrep = new AODVRouteResponse("AODV-RREP",RREP);
                rrep->setRouteDestAddr(fwdRoute->destAddr);
                rrep->setRouteDestSeqNo(fwdRoute->destSeqNo);
                rrep->setRouteSrcAddr(rreq->getInitialSrcAddr());
                rrep->setHopCount(fwdRoute->hopCount);

                routeMapElement* revRoute = getRouteForDestination(rreq->getInitialSrcAddr());
                rrep->setDestAddr(revRoute->nextHop);
                rrep->setSrcAddr(myNetwAddr);

                NetwToMacControlInfo::setControlInfo(rrep, arp->getMacAddr(revRoute->nextHop));
                sendDown(rrep);
            }
        }
        //destination
        else if (rreq->getFinalDestAddr()==myNetwAddr)
        {
            debugEV << "FINAL DEST NODE - received a RREQ from node: " << rreq->getInitialSrcAddr() << endl;
            upsertReverseRoute(rreq);

            nodeSeqNo++;
            AODVRouteResponse* rrep = new AODVRouteResponse("AODV-RREP",RREP);
            rrep->setRouteDestAddr(myNetwAddr);
            rrep->setRouteDestSeqNo(nodeSeqNo);
            rrep->setRouteSrcAddr(rreq->getInitialSrcAddr());
            rrep->setHopCount(0);

            routeMapElement* revRoute = getRouteForDestination(rreq->getInitialSrcAddr());
            rrep->setDestAddr(revRoute->nextHop);
            rrep->setSrcAddr(myNetwAddr);

            EV << "macAddress:" << arp->getMacAddr(revRoute->nextHop) << endl;
            NetwToMacControlInfo::setControlInfo(rrep, arp->getMacAddr(revRoute->nextHop));
            sendDown(rrep);
            delete msg;
        }
        //source - ignore
        else
        {
            debugEV << "SRC NODE - received a RREQ from node: " << rreq->getSrcAddr() << endl;
            delete msg;
        }
    }
    else //have received it already. ignore
    {
        delete msg;
    }
}
void AODVRoute::handleUpperControlHasRoute(ApplPkt* ctrlPkt)
{
    debugEV << "HAS_ROUTE(?) received for destAddr: " << ctrlPkt->getDestAddr() << endl;
    if (hasRouteForDestination(ctrlPkt->getDestAddr()) || LAddress::isL3Broadcast(ctrlPkt->getDestAddr()))
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

        nodeSeqNo++;
        AODVRouteRequest* pkt = new AODVRouteRequest("AODV-RREQ",RREQ);
        pkt->setInitialSrcAddr(myNetwAddr);
        pkt->setFinalDestAddr(ctrlPkt->getDestAddr());
        pkt->setSrcAddr(myNetwAddr);
        pkt->setDestAddr(LAddress::L3BROADCAST);
        pkt->setRREQ_ID(RREQIDCounter);
        if (!getNodeSeqNo(pkt->getFinalDestAddr()))
        {
            upsertNodeSeqNo(pkt->getFinalDestAddr(),0); //unknown seqno at destinatino node
        }
        pkt->setFinalDestSeqNo(getNodeSeqNo(pkt->getFinalDestAddr()));
        pkt->setInitialSrcSeqNo(nodeSeqNo);
        pkt->setHopCount(0);

        NetwToMacControlInfo::setControlInfo(pkt,LAddress::L2BROADCAST);
        RREQIDCounter++;

        sendDown(pkt);
    }
}


/// OTHER NODES last know seqNo
int AODVRoute::getNodeSeqNo(LAddress::L3Type addr)
{
    std::map<LAddress::L3Type,int>::iterator it = nodesLastKnownSeqNoMap.find(addr);
    if (it!=nodesLastKnownSeqNoMap.end())
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
    std::map<LAddress::L3Type,int>::iterator it = nodesLastKnownSeqNoMap.find(addr);
    if (it==nodesLastKnownSeqNoMap.end())
    {
        nodesLastKnownSeqNoMap[addr] = seqNumber;
    }
    else
    {
        if (seqNumber > it->second)
        {
            nodesLastKnownSeqNoMap[it->first] = seqNumber;
        }
        else
        {
            debugEV << "seqNo discarded because it is less or equal than the existing one" << endl;
        }
    }
}

//ROUTES TABLE
AODVRoute::routeMapElement_t* AODVRoute::getRouteForDestination(LAddress::L3Type destAddr)
{
    routeMap_t::iterator it = routeMap.find(destAddr);
    return (it!=routeMap.end() && it->second->lifeTime>=simTime())?it->second:NULL;
}
bool AODVRoute::hasRouteForDestination(LAddress::L3Type destAddr)
{
    return (getRouteForDestination(destAddr)!=NULL);
}
bool AODVRoute::upsertReverseRoute(AODVRouteRequest* msg)
{
    debugEV << "Updating REVERSE Route for node " << msg->getInitialSrcAddr() << endl;
    routeMapElement* tEl = (struct routeMapElement*) malloc(sizeof(struct routeMapElement));
    tEl->destAddr = msg->getInitialSrcAddr();
    tEl->destSeqNo = msg->getInitialSrcSeqNo();
    tEl->hopCount = msg->getHopCount();
    tEl->lifeTime = simTime()+routeMapElementMaxLifetime;
    tEl->nextHop = msg->getSrcAddr();
    return upsertRoute(tEl);
}
bool AODVRoute::upsertForwardRoute(AODVRouteResponse* msg)
{
    debugEV << "Updating FORWARD Route for node " << msg->getRouteDestAddr() << endl;
    routeMapElement* tEl = (struct routeMapElement*) malloc(sizeof(struct routeMapElement));
    tEl->destAddr = msg->getRouteDestAddr();
    tEl->destSeqNo = msg->getRouteDestSeqNo();
    tEl->hopCount = msg->getHopCount();
    tEl->lifeTime = simTime()+routeMapElementMaxLifetime;
    tEl->nextHop = msg->getSrcAddr();
    return upsertRoute(tEl);
}
bool AODVRoute::upsertRoute(routeMapElement* rtEl)
{
    //only updates(delete+insert) or inserts if has the one of the three conditions
    // 1 - route doesn't exist
    // 2 - route has expired
    // 3 - route exists but is older
    // 4 - route is the same age but has less hops
    routeMapElement* currentRoute = getRouteForDestination(rtEl->destAddr);
    if ((currentRoute==NULL) ||
       (currentRoute->lifeTime < simTime()) ||
       (currentRoute!=NULL && currentRoute->destSeqNo<rtEl->destSeqNo) ||
       (currentRoute!=NULL && currentRoute->destSeqNo==rtEl->destSeqNo && currentRoute->hopCount > rtEl->hopCount))
    {
        debugEV << "Route for destAddr:" << rtEl->destAddr << " UPSERTED! " << endl;
        routeMap.erase(rtEl->destAddr);
        routeMap.insert(std::pair<LAddress::L3Type,routeMapElement*>(rtEl->destAddr,rtEl));
        return true;
    }
    return false;
}
AODVRoute::unreachAddSeqNoMap_t AODVRoute::removeRoutesByNextHop(LAddress::L3Type nextHop)
{
    routeMap_t::iterator it;
    unreachAddSeqNoMap_t addrMap;

    for (it=routeMap.begin();it!=routeMap.end();it++)
    {
        if (it->second->nextHop == nextHop)
        {
            addrMap[it->second->destAddr] = it->second->destSeqNo;
        }
    }
    return addrMap;
}
void AODVRoute::routeTableMaintenance()
{
    //TODO checks for expired elements
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
    debugEV << "Saving the RREQ! ID:" << rreq->getRREQ_ID() << " , srcAddr: " << rreq->getInitialSrcAddr() << endl;
    RREQVectorElement* tEl = (struct RREQVectorElement*) malloc(sizeof(struct RREQVectorElement));
    tEl->srcAddr = rreq->getInitialSrcAddr();
    tEl->RREQ_ID = rreq->getRREQ_ID();
    tEl->lifeTime = simTime()+RREQVectorElementMaxLifetime;
    while (RREQVector.size()>=RREQVectorMaxSize)
    {
        RREQVector.erase(RREQVector.begin());
    }
    RREQVector.push_back(tEl);
}
void AODVRoute::runRREQSetMaintenance()
{
    //TODO clear all entries that have expired
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
    //adds RSSI and initialSrcAddress info for AppLayer
    NetwToApplControlInfo::setControlInfo(pkt,cInfo->getRSSI(), msg->getInitialSrcAddr());

    delete msg;
    return pkt;
}

///AUX
std::string AODVRoute::serializeAddressSeqNoMap(unreachAddSeqNoMap_t map)
{
    //TODO
    unreachAddSeqNoMap_t::iterator it;
    for (it=map.begin();it!=map.end();it++)
    {

    }
}
