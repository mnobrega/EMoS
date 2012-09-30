#ifndef AODV_ROUTE_H
#define AODV_ROUTE_H

#include <omnetpp.h>
#include <limits>
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "ApplPkt_m.h"
#include "AODVData_m.h"
#include "AODVRouteRequest_m.h"
#include "AODVRouteResponse_m.h"
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
    int RREQSent;
    int nodeSeqNo;

    //other nodes last known seq numbers
    std::map<LAddress::L3Type,int> nodesLastKnownSeqNoTable;

    //route table
    unsigned int maxRouteTableElementLifetime;
    struct routeTableElement
    {
        LAddress::L3Type destAddr;
        //sequence number at the destination node when it sent a RREP
        int destSeqNo;
        LAddress::L3Type nextHop;
        int hopCount;
        simtime_t lifeTime;
    };
    typedef routeTableElement routeTableElement_t;
    std::map<LAddress::L3Type,routeTableElement*> routeTable;

    //route req table
    unsigned int maxRREQVectorSize;
    unsigned int maxRREQVectorElementLifetime;
    unsigned int RREQVectorMaintenancePeriod;
    struct RREQVectorElement
    {
        int     RREQ_ID;
        LAddress::L3Type srcAddr;
        simtime_t lifeTime;
    };
    std::vector<RREQVectorElement*> RREQVector;

    void handleUpperMsg(cMessage *);
    void handleLowerMsg(cMessage *);
    void handleSelfMsg(cMessage *);
    void handleLowerControl(cMessage * msg);
    void handleUpperControl(cMessage * msg);
    AODVData* encapsMsg(cPacket *);
    cPacket* decapsMsg(AODVData *);

    void handleUpperControlHasRoute(ApplPkt*);
    void handleLowerRREQ(cMessage*);

    int getNodeSeqNo(LAddress::L3Type);
    void upsertNodeSeqNo(LAddress::L3Type,int);

    routeTableElement_t* getRouteForDestination(LAddress::L3Type);
    bool hasRouteForDestination(LAddress::L3Type);
    void routeTableMaintenance();
    bool upsertReverseRoute(AODVRouteRequest*);
    bool upsertForwardRoute(AODVRouteResponse*);
    bool upsertRoute(routeTableElement*);

    bool hasRREQ(AODVRouteRequest*);
    void saveRREQ(AODVRouteRequest*);
    void runRREQSetMaintenance();

};

#endif // AODV_ROUTE_H
