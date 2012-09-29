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
	}
	else if (stage == 1)
	{
	    debugEV << "Host index=" << findHost()->getIndex() << ", Id=" << findHost()->getId() << endl;
	    debugEV << "  host IP address=" << myNetwAddr << endl;
	    debugEV << "  host macaddress=" << arp->getMacAddr(myNetwAddr) << endl;
	}
}

void AODVRoute::finish()
{
    if (stats)
    {
        recordScalar("Aodv totalSend ", totalSend);
        recordScalar("rreq send", totalRreqSend);
        recordScalar("rreq rec", totalRreqRec);
        recordScalar("rrep send", totalRrepSend);
        recordScalar("rrep rec", totalRrepRec);
        recordScalar("rerr send", totalRerrSend);
        recordScalar("rerr rec", totalRerrRec);
    }
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
void AODVRoute::handleLowerMsg(cMessage* msg)
{
    NetwPkt* m = static_cast<NetwPkt*>(msg);

    switch (msg->getKind())
    {
        case DATA:
            {
                ApplPkt* pkt = static_cast<ApplPkt*>(decapsMsg(m));
                sendUp(pkt);
            }
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void AODVRoute::handleUpperMsg(cMessage * msg)
{
    ApplPkt* appPkt = static_cast<ApplPkt*>(msg);
    LAddress::L3Type destAddress = NetwControlInfo::getAddressFromControlInfo(appPkt->getControlInfo());

    if (destAddress==LAddress::L3BROADCAST || hasRouteForDestination(destAddress))
    {
        NetwPkt* netwPkt = encapsMsg(appPkt);
        debugEV << "route found for destination:" << destAddress << endl;
        sendDown(netwPkt);
    }
    else
    {
        debugEV << "route not found! data msg sending fail! for destination:" << destAddress << endl;
        delete appPkt;
        //send msg fail and control message to app layer
    }
}
void AODVRoute::handleUpperControl(cMessage* msg)
{
    ApplPkt* ctrlPkt = static_cast<ApplPkt*>(msg);
    switch (msg->getKind())
    {
        case HAS_ROUTE:
            debugEV << "HAS_ROUTE received for destAddr: " << ctrlPkt->getDestAddr() << endl;
            if (hasRouteForDestination(ctrlPkt->getDestAddr()))
            {

            }
            else
            {
                //getRoute
            }
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
}


/// ROUTE TABLE
bool AODVRoute::hasRouteForDestination(LAddress::L3Type destAddr)
{
    return false;
}
void AODVRoute::checkRouteTable()
{
    //todo - checks the route table for routes with expired lifetime
}

/// ENCAPSULATION
NetwPkt* AODVRoute::encapsMsg(cPacket* cPkt)
{
    ApplPkt* appPkt = static_cast<ApplPkt*>(cPkt);
    NetwPkt* pkt = new NetwPkt(appPkt->getName(), DATA);
    cObject* cInfo = appPkt->removeControlInfo();

    LAddress::L3Type netwDestAddr = NetwControlInfo::getAddressFromControlInfo(cInfo);
    LAddress::L2Type macDestAddr = (LAddress::isL3Broadcast(netwDestAddr))?(LAddress::L2BROADCAST):(arp->getMacAddr(netwDestAddr));

    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwDestAddr);
    pkt->setBitLength(headerLength);
    pkt->encapsulate(appPkt);
    NetwToMacControlInfo::setControlInfo(pkt, macDestAddr);

    return pkt;
}
cPacket* AODVRoute::decapsMsg(NetwPkt *msg)
{
    ApplPkt* pkt = static_cast<ApplPkt*>(msg->decapsulate());
    MacToNetwControlInfo* cInfo = static_cast<MacToNetwControlInfo*>(msg->removeControlInfo());
    NetwToApplControlInfo::setControlInfo(pkt,cInfo->getRSSI(), msg->getSrcAddr());

    delete msg;
    return pkt;
}
