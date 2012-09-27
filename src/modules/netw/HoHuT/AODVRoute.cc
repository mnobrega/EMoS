#include "AODVRoute.h"

#include <limits>
#include <algorithm>
#include <cassert>

#include "HoHuTApplPkt_m.h"
#include "WiseRoutePkt_m.h"
#include "ArpInterface.h"
#include "MacToNetwControlInfo.h"
#include "NetwControlInfo.h"

using std::endl;

Define_Module(AODVRoute);

void AODVRoute::initialize(int stage)
{
	BaseNetwLayer::initialize(stage);
	if (stage == 0)
	{
		trace = par("trace");
	}
	else if (stage == 1)
	{
	    EV << "Host index=" << findHost()->getIndex() << ", Id=" << findHost()->getId() << endl;
	    EV << "  host IP address=" << myNetwAddr << endl;
	    EV << "  host macaddress=" << arp->getMacAddr(myNetwAddr) << endl;
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
    HoHuTApplPkt* m = static_cast<HoHuTApplPkt*>(msg);
	sendDown(encapsMsg(m));
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

NetwPkt* AODVRoute::encapsMsg(cPacket *appPkt)
{
    LAddress::L2Type macAddr;
    LAddress::L3Type netwAddr;

    WiseRoutePkt *pkt = new WiseRoutePkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setSrcAddr(myNetwAddr);

    cObject* cInfo = appPkt->removeControlInfo();

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
    pkt->setFinalDestAddr(netwAddr);

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

cMessage* AODVRoute::decapsMsg(NetwPkt *msg)
{
    HoHuTApplPkt *pkt = static_cast<HoHuTApplPkt*>(msg->decapsulate());
    setUpControlInfo(pkt, msg->getSrcAddr());
    delete msg;
    return pkt;
}
