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
        packetMapMaxPktQueueElementLifetime = par("packetMapMaxPktQueueElementLifetime");
        packetMapMaintenancePeriod = par("packetMapMaintenancePeriod");
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
    destroyPktMap();
}


/// MSG HANDLING
void AODVRoute::handleSelfMsg(cMessage * msg)
{
   delete msg; //no self-messages for now
}
//FROM APPL
void AODVRoute::handleUpperMsg(cMessage * msg)
{
    handleUpperApplPkt(msg);
}
void AODVRoute::handleUpperControl(cMessage* msg)
{
    delete msg; //no upper ctrl messages for now
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
        case RERR:
            handleLowerRERR(msg);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void AODVRoute::handleLowerControl(cMessage* msg)
{
    switch(msg->getKind())
    {
        case PACKET_DROPPED:
            handleLowerControlPacketDropped(msg);
            break;
        case TX_OVER:
            handleLowerControlTXOver(msg);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}



/// HANDLE MSG TYPES
void AODVRoute::handleUpperApplPkt (cMessage* msg)
{
    ApplPkt* appPkt = static_cast<ApplPkt*>(msg);
    NetwControlInfo* cInfo = static_cast<NetwControlInfo*>(appPkt->removeControlInfo());
    LAddress::L3Type destAddress = cInfo->getNetwAddr();

    if (LAddress::isL3Broadcast(destAddress))
    {
        debugEV << "Broadcasting ApplPkt" << endl;
        AODVData* data = new AODVData(msg->getName(),DATA);
        data->setFinalDestAddr(LAddress::L3BROADCAST);
        data->setInitialSrcAddr(myNetwAddr);
        data->setSrcAddr(myNetwAddr);
        data->setDestAddr(LAddress::L3BROADCAST);
        data->encapsulate(appPkt);
        NetwToMacControlInfo::setControlInfo(data,LAddress::L2BROADCAST);
        sendDown(data);
    }
    else if (hasRouteForDestination(destAddress))
    {
        debugEV << "Route found for destAddr: " << destAddress << endl;

        AODVData* data = new AODVData(msg->getName(),DATA);
        routeMapElement* fwdRoute = getRouteForDestination(destAddress);
        data->setFinalDestAddr(destAddress);
        data->setInitialSrcAddr(myNetwAddr);
        data->setSrcAddr(myNetwAddr);
        data->setDestAddr(fwdRoute->nextHop);
        data->encapsulate(appPkt);
        NetwToMacControlInfo::setControlInfo(data,arp->getMacAddr(fwdRoute->nextHop));
        sendDown(data);
    }
    else if (destAddress!=myNetwAddr) //try to find route
    {
        debugEV << "Route NOT FOUND! Sending RREQ for destAddr: " << destAddress << endl;

        addToPktMap(appPkt,destAddress);

        nodeSeqNo++;
        RREQIDCounter++;
        AODVRouteRequest* rreq = new AODVRouteRequest("AODV-RREQ",RREQ);
        rreq->setInitialSrcAddr(myNetwAddr);
        rreq->setFinalDestAddr(destAddress);
        rreq->setSrcAddr(myNetwAddr);
        rreq->setDestAddr(LAddress::L3BROADCAST);
        rreq->setRREQ_ID(RREQIDCounter);
        if (!getNodeSeqNo(rreq->getFinalDestAddr()))
        {
            upsertNodeSeqNo(rreq->getFinalDestAddr(),0); //unknown seqno at destinatino node
        }
        rreq->setFinalDestSeqNo(getNodeSeqNo(destAddress));
        rreq->setInitialSrcSeqNo(nodeSeqNo);
        rreq->setHopCount(0);

        NetwToMacControlInfo::setControlInfo(rreq,LAddress::L2BROADCAST);
        sendDown(rreq);
    }
    else
    {
        delete msg;
    }
}
void AODVRoute::handleLowerRERR(cMessage* msg)
{
    addressSeqNoMap_t::iterator it;
    routeSet_t* removedRoutes;

    AODVRouteError* rerr = static_cast<AODVRouteError*>(msg);
    LAddress::L3Type srcAddr = rerr->getSrcAddr();
    addressSeqNoMap_t uDests = rerr->getUnreachDestAddress();

    for (it=uDests.begin(); it!=uDests.end(); it++)
    {
        removedRoutes = removeRoutesByDestinationAndNextHop(it->first,srcAddr);
        sendRERRByRemovedRoutes(removedRoutes);
    }

    delete msg;
}
void AODVRoute::handleLowerDATA(cMessage* msg)
{
    AODVData* data = static_cast<AODVData*>(msg);
    MacToNetwControlInfo* cInfo = static_cast<MacToNetwControlInfo*>(data->removeControlInfo());

    if (data->getFinalDestAddr()!=myNetwAddr && data->getInitialSrcAddr()!=myNetwAddr && !LAddress::isL3Broadcast(data->getDestAddr())) //intermediate node and NOT BROADCAST
    {
        addressSet_t::iterator it;
        routeMapElement* fwdRoute = getRouteForDestination(data->getFinalDestAddr());

        it=fwdRoute->precursors->find(data->getSrcAddr());
        if (it==fwdRoute->precursors->end())
        {
            fwdRoute->precursors->insert(data->getSrcAddr());
        }

        data->setDestAddr(fwdRoute->nextHop);
        data->setSrcAddr(myNetwAddr);
        NetwToMacControlInfo::setControlInfo(data,arp->getMacAddr(fwdRoute->nextHop));

        sendDown(data);
    }
    else if (data->getFinalDestAddr()==myNetwAddr || LAddress::isL3Broadcast(data->getDestAddr())) //DESTINATION
    {
        ApplPkt* applPkt = static_cast<ApplPkt*>(data->decapsulate());
        NetwToApplControlInfo::setControlInfo(applPkt,cInfo->getRSSI(),data->getInitialSrcAddr());
        sendUp(applPkt);
        delete msg;
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
    rrep->removeControlInfo();

    //intermediate node
    if (rrep->getRouteDestAddr()!=myNetwAddr && rrep->getRouteSrcAddr()!=myNetwAddr)
    {
        debugEV << "Arrived to INTERMEDIATE NODE! Save fwd route and rev msg" << endl;
        upsertForwardRoute(rrep);
        routeMapElement* revRoute = getRouteForDestination(rrep->getRouteSrcAddr());
        rrep->setSrcAddr(myNetwAddr);
        rrep->setDestAddr(revRoute->nextHop);
        NetwToMacControlInfo::setControlInfo(rrep,arp->getMacAddr(revRoute->nextHop));
        sendDown(rrep);
    }
    //route source node
    else if (rrep->getRouteSrcAddr()==myNetwAddr)
    {
        debugEV << "Arrived to ROUTE SOURCE! Completed ROUTE discovery!!!" << endl;
        upsertForwardRoute(rrep);
        LAddress::L3Type destAddr = rrep->getRouteDestAddr();

        debugEV << "Sending app pkt to destination addr: " << destAddr << endl;
        ApplPkt* appPkt = getFromPktMap(destAddr);
        AODVData* data = new AODVData("aodv-data",DATA);
        routeMapElement* fwdRoute = getRouteForDestination(destAddr);
        data->setFinalDestAddr(destAddr);
        data->setInitialSrcAddr(myNetwAddr);
        data->setSrcAddr(myNetwAddr);
        data->setDestAddr(fwdRoute->nextHop);
        data->encapsulate(appPkt);
        NetwToMacControlInfo::setControlInfo(data,arp->getMacAddr(fwdRoute->nextHop));
        sendDown(data);
        delete msg;
    }
    //route destination node - ignore
    else if (rrep->getRouteDestAddr()==myNetwAddr)
    {
        debugEV << "Arrived to ROUTE DESTINATION! Ignore the msg!";
        delete msg;
    }
    else
    {
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
            if (upsertReverseRoute(rreq))
            {
                nodeSeqNo++;
                AODVRouteResponse* rrep = new AODVRouteResponse("AODV-RREP",RREP);
                rrep->setRouteDestAddr(myNetwAddr);
                rrep->setRouteDestSeqNo(nodeSeqNo);
                rrep->setRouteSrcAddr(rreq->getInitialSrcAddr());
                rrep->setHopCount(0);

                routeMapElement* revRoute = getRouteForDestination(rreq->getInitialSrcAddr());
                rrep->setDestAddr(revRoute->nextHop);
                rrep->setSrcAddr(myNetwAddr);

                NetwToMacControlInfo::setControlInfo(rrep, arp->getMacAddr(revRoute->nextHop));
                sendDown(rrep);
            }
            else
            {
                debugEV << "The rreq was worse than the existing one! Dont upsert!" << endl;
            }
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
void AODVRoute::handleLowerControlTXOver(cMessage* msg)
{
    debugEV << "Transmission sucess received!" << endl;
    MacPkt* macPkt = static_cast<MacPkt*>(msg);
    NetwPkt* netwPkt = static_cast<NetwPkt*>(macPkt->decapsulate());
    delete msg;
    delete netwPkt;
}
void AODVRoute::handleLowerControlPacketDropped (cMessage * msg)
{
    debugEV << "Transmission failed received!" << endl;
    MacPkt* macPkt = static_cast<MacPkt*>(msg);
    NetwPkt* netwPkt = static_cast<NetwPkt*>(macPkt->decapsulate());
    if (netwPkt->getKind()==DATA)
    {
        if (!localRepair)
        {
            AODVData* aodvData = static_cast<AODVData*>(netwPkt);
            routeSet_t* removedRoutes = removeRoutesByNextHop(aodvData->getDestAddr());
            sendRERRByRemovedRoutes(removedRoutes);
            debugEV << "Packet for dest: " << aodvData->getFinalDestAddr() << " sent by src: " << aodvData->getInitialSrcAddr() << " was LOST!" << endl;
            bubble("PACKET LOST");
        }
        else
        {
            //TODO (not implement for now)
        }
    }
    delete msg;
    delete netwPkt;
}
void AODVRoute::sendRERRByRemovedRoutes(routeSet_t* removedRoutes)
{
    routeSet_t::iterator it;
    addressSet_t::iterator it2;
    addressSeqNoMap_t unreachDestinations;

    for (it=removedRoutes->begin(); it!=removedRoutes->end();it++)
    {
        routeMapElement_t* routeMapEl = *it;
        unreachDestinations.insert(std::pair<LAddress::L3Type,int>(routeMapEl->destAddr,nodesLastKnownSeqNoMap[routeMapEl->destAddr]));
    }
    for (it=removedRoutes->begin(); it!=removedRoutes->end();it++)
    {
        routeMapElement_t* routeMapEl = *it;
        addressSet_t* precursors = routeMapEl->precursors;
        for (it2=precursors->begin(); it2!=precursors->end();it2++)
        {
            LAddress::L3Type precursorAddress = *it2;
            AODVRouteError* rerr = new AODVRouteError("route-error",RERR);
            rerr->setUnreachDestAddress(unreachDestinations);
            rerr->setSrcAddr(myNetwAddr);
            rerr->setDestAddr(precursorAddress);
            NetwToMacControlInfo::setControlInfo(rerr,precursorAddress);

            sendDown(rerr);

            debugEV << "sending RERR to precurssor address: " << precursorAddress << endl;
        }
    }
}


/// OTHER NODES last know seqNo
int AODVRoute::getNodeSeqNo(LAddress::L3Type addr)
{
    addressSeqNoMap_t::iterator it = nodesLastKnownSeqNoMap.find(addr);
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
    addressSeqNoMap_t::iterator it = nodesLastKnownSeqNoMap.find(addr);
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
void AODVRoute::resetRouteLifeTime(LAddress::L3Type destAddr)
{
    debugEV << "Route lifetime extended because the route to :" << destAddr << " was used!" << endl;
    routeMapElement_t* route = getRouteForDestination(destAddr);
    route->lifeTime = simTime()+routeMapElementMaxLifetime;
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
    tEl->precursors = new addressSet_t;
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
    tEl->precursors = new addressSet_t;
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
AODVRoute::routeSet_t* AODVRoute::removeRoutesByNextHop(LAddress::L3Type nextHop)
{
    routeMap_t::iterator it;
    routeSet_t* removedRoutes = new routeSet_t;

    for (it=routeMap.begin();it!=routeMap.end();it++)
    {
        if (it->second->nextHop == nextHop)
        {
            removedRoutes->insert(it->second);
            routeMap.erase(it);
            nodesLastKnownSeqNoMap[it->second->destAddr]++;
        }
    }
    return removedRoutes;
}
AODVRoute::routeSet_t* AODVRoute::removeRoutesByDestinationAndNextHop(LAddress::L3Type dest, LAddress::L3Type nextHop)
{
    routeMap_t::iterator it;
    routeSet_t* removedRoutes = new routeSet_t;

    for (it=routeMap.begin();it!=routeMap.end();it++)
    {
        if (it->second->destAddr==dest && it->second->nextHop==nextHop)
        {
            removedRoutes->insert(it->second);
            routeMap.erase(it);
            nodesLastKnownSeqNoMap[it->second->destAddr]++;
        }
    }
    return removedRoutes;
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


/// PACKET MAP
void AODVRoute::destroyPktMap()
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
           ApplPkt* pkt = static_cast<ApplPkt*>(qEl->packet);
           delete pkt;
           count++;
       }
       debugEV << count << " pkts were destroyed for destAddr:" << it->first << endl;
    }

}
void AODVRoute::addToPktMap(ApplPkt * pkt, LAddress::L3Type destAddr)
{
    pktQueueElement* qEl = (struct pktQueueElement *) malloc(sizeof(struct pktQueueElement));

    qEl->destAddr = destAddr;
    qEl->lifeTime = simTime()+packetMapMaxPktQueueElementLifetime;
    qEl->packet = pkt;

    debugEV << "Adding packet to map for address :" << destAddr << endl;

    pktMap_t::iterator it = pktMap.find(destAddr);
    if (it!=pktMap.end())
    {
        it->second.push(qEl);
    }
    else
    {
        pktQueue_t pktQueue;
        pktQueue.push(qEl);
        pktMap.insert(std::pair<LAddress::L3Type,pktQueue_t>(destAddr,pktQueue));
    }
}
ApplPkt* AODVRoute::getFromPktMap(LAddress::L3Type destAddr)
{
    pktMap_t::iterator it = pktMap.find(destAddr);
    if (it!=pktMap.end())
    {
        if (it->second.size()>0)
        {
            pktQueueElement* qEl = it->second.front();
            it->second.pop();
            ApplPkt* pkt = static_cast<ApplPkt*>(qEl->packet);
            if (it->second.size()==0)
            {
                pktMap.erase(it);
            }
            return pkt;
        }
    }
    return NULL;
}
void AODVRoute::runPktMapMaintenance()
{
    debugEV << "Checking packet queue for packets to send or expire!" << endl;
}
