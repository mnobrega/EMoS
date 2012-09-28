#include "AODVRoute.h"

#include <limits>
#include <algorithm>
#include <cassert>

// todo delete
#include "HoHuTApplPkt_m.h"
#include "WiseRoutePkt_m.h"
// todo delete

#include "ArpInterface.h"
#include "MacToNetwControlInfo.h"
#include "NetwControlInfo.h"
#include "ApplPkt_m.h"

using std::endl;

Define_Module(AODVRoute);

void AODVRoute::initialize(int stage)
{
	BaseNetwLayer::initialize(stage);
	if (stage == 0)
	{
		trace = par("trace");
		debug = par("debug").boolValue();
		pktQueueElementLifetime = par("pktQueueElementLifetime");
		pktQueueCheckingPeriod = par("pktQueueCheckingPeriod");
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
    destroyPktQueue(); //had error. not prioritary so commented
}

/// MSG HANDLING
void AODVRoute::handleSelfMsg(cMessage * msg)
{
   switch (msg->getKind())
   {
       case CHECK_PKT_QUEUE_TIMER:
           checkPktQueue();
           delete msg;
           break;
       default:
           delete msg;
           break;
   }
}
void AODVRoute::handleLowerMsg(cMessage* msg)
{
    if (msg->getKind()==DATA)
    {
        WiseRoutePkt* m = static_cast<WiseRoutePkt*>(msg);
        const LAddress::L3Type& finalDestAddr  = m->getFinalDestAddr();
        const LAddress::L3Type& initialSrcAddr = m->getInitialSrcAddr();
        double rssi = static_cast<MacToNetwControlInfo*>(m->getControlInfo())->getRSSI();

        if (finalDestAddr==myNetwAddr || LAddress::isL3Broadcast(finalDestAddr))
        {
            HoHuTApplPkt* decapsulatedMsg = static_cast<HoHuTApplPkt*>(decapsMsg(m));
            decapsulatedMsg->setSignalStrength(rssi);
            decapsulatedMsg->setSrcAddr(initialSrcAddr);
            sendUp(decapsulatedMsg);
        }
    }
    else
    {
        delete msg;
    }
}
void AODVRoute::handleUpperMsg(cMessage * msg)
{
    ApplPkt* appPkt = static_cast<ApplPkt*>(msg);
    NetwPkt* netwPkt = encapsMsg(appPkt);

    if (hasRouteForDestination(netwPkt->getDestAddr()))
    {
        debugEV << "route found for destination:" << netwPkt->getDestAddr() << endl;
        sendDown(netwPkt);
    }
    else
    {
        debugEV << "route not found for destination:" << netwPkt->getDestAddr() << endl;
        cMessage* selfTimer = new cMessage("beacon-timer",CHECK_PKT_QUEUE_TIMER);
        scheduleAt(simTime() + pktQueueCheckingPeriod, selfTimer);
        addToPktQueue(netwPkt);

        //find route
    }
}

/// ENCAPSULATION
NetwPkt* AODVRoute::encapsMsg(cPacket* appPkt)
{
    LAddress::L2Type macAddr;
    LAddress::L3Type netwAddr;

    cObject* cInfo = appPkt->removeControlInfo();

    NetwPkt *pkt = new NetwPkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);
    pkt->setSrcAddr(myNetwAddr);

    if(cInfo == NULL)
    {
        netwAddr = LAddress::L3BROADCAST;
    }
    else
    {
        netwAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
        delete cInfo;
    }
    pkt->setDestAddr(netwAddr);

    if(LAddress::isL3Broadcast(netwAddr))
    {
        macAddr = LAddress::L2BROADCAST;
    }
    else
    {
        macAddr = arp->getMacAddr(netwAddr);
    }
    setDownControlInfo(pkt, macAddr);

    pkt->encapsulate(appPkt);
    return pkt;
}
cPacket* AODVRoute::decapsMsg(NetwPkt *msg)
{
    ApplPkt *pkt = static_cast<ApplPkt*>(msg->decapsulate());
    delete msg;
    return pkt;
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


/// PACKET QUEUE
void AODVRoute::destroyPktQueue()
{
    int count = 0;
    while (!pktQueue.empty())
    {
        pktQueueElement *qEl = pktQueue.back();
        pktQueue.pop_back();
        EV << qEl->destAddr;
        EV << qEl->lifeTime;
        EV << qEl->packet->getDestAddr();
        //delete decapsMsg(qEl->packet);
        //delete qEl->packet;
        //free(qEl);
        count++;
    }
    debugEV << count << " pkts were destroyed" << endl;
}
void AODVRoute::addToPktQueue(NetwPkt * pkt)
{
    debugEV << "Adding packet to queue!" << endl;
    pktQueueElement qEl;
    qEl.destAddr = pkt->getDestAddr();
    qEl.lifeTime = simTime()+pktQueueElementLifetime;
    qEl.packet = pkt;
    pktQueue.push_back(&qEl);
}
void AODVRoute::checkPktQueue()
{
    debugEV << "Checking packet queue for packets to send or expire!" << endl;
}


