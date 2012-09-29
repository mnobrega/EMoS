#include <omnetpp.h>
#include <limits>
#include <algorithm>
#include <cassert>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "ApplPkt_m.h"
#include "NetwPkt_m.h"
#include "AODVRouteRequest_m.h"
#include "ArpInterface.h"
#include "MacToNetwControlInfo.h"
#include "NetwToMacControlInfo.h"
#include "NetwControlInfo.h"
#include "NetwToApplControlInfo.h"

class MIXIM_API AODVRoute : public BaseNetwLayer
{
public:
    virtual void initialize(int);
    virtual void finish();

protected:
    enum AODV_MSG_TYPES
    {
        RREQ,
        RREP,
        RERR,
        DATA
    };
    enum AODV_CTRL_MSG_TYPES
    {
        HAS_ROUTE,
        HAS_ROUTE_ACK,
        DELIVERY_ACK,
        DELIVERY_ERROR
    };
    enum BaseMacControlKinds
    {
        TX_OVER = 23500,
        PACKET_DROPPED,
        LAST_BASE_MAC_CONTROL_KIND
    };

    bool stats, trace;

    int totalSend;
    int totalRreqSend;
    int totalRreqRec;
    int totalRrepSend;
    int totalRrepRec;
    int totalRerrSend;
    int totalRerrRec;

    void handleUpperMsg(cMessage *);
    void handleLowerMsg(cMessage *);
    void handleSelfMsg(cMessage *);
    void handleLowerControl(cMessage * msg);
    void handleUpperControl(cMessage * msg);
    NetwPkt* encapsMsg(cPacket *);
    cPacket* decapsMsg(NetwPkt *);

    //route table
    struct routeTableEntry
    {
        LAddress::L3Type destAddr;
        int destSeqNo;
        LAddress::L3Type nextHop;
        int hopCount;
        int lifeTime;
    };
    std::map<LAddress::L3Type,routeTableEntry> routeTable;
    void getRoute(LAddress::L3Type destAddr);
    bool hasRouteForDestination(LAddress::L3Type);
    void checkRouteTable();
};
