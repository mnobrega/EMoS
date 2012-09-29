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
        case RREQ:
            {
                AODVRouteRequest* pkt = static_cast<AODVRouteRequest*>(decapsMsg(m));
                debugEV << "received a RREQ from node:" << pkt->getSrcAddr() << endl;
                delete msg;
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
        debugEV << "using route and sending down to destAddr:" << destAddress << endl;
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
                debugEV << "Route found for destAddr: " << ctrlPkt->getDestAddr() << endl;
                ApplPkt* ctrlPktResponse = new ApplPkt("has-route-ACK",HAS_ROUTE_ACK);
                ctrlPktResponse->setDestAddr(ctrlPkt->getDestAddr());
                sendControlUp(ctrlPktResponse);
            }
            else
            {
                debugEV << "Route not found for destAddr: " << ctrlPkt->getDestAddr() << endl;
                debugEV << "Sending RREQ" << endl;
                AODVRouteRequest* pkt = new AODVRouteRequest("RREQ",RREQ);
                pkt->setDestAddr(ctrlPkt->getDestAddr());
                pkt->setSrcAddr(myNetwAddr);
                pkt->setDestSeqNo(0);
                pkt->setSrcSeqNo(0);
                pkt->setHopCount(0);
                NetwToMacControlInfo::setControlInfo(pkt,LAddress::L2BROADCAST);
                sendDown(pkt);
            }
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            break;
    }
    delete msg;
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
