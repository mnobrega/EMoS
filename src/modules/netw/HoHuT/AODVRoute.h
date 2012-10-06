#ifndef AODV_ROUTE_H
#define AODV_ROUTE_H

#include <omnetpp.h>
#include <limits>
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <string>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "ApplPkt_m.h"
#include "MacPkt_m.h"
#include "AODVData_m.h"
#include "AODVRouteRequest_m.h"
#include "AODVRouteResponse_m.h"
#include "AODVRouteError_m.h"
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
        DELIVERY_ERROR,
        DELIVERY_LOCAL_REPAIR
    };
    enum BaseMacControlKinds
    {
        TX_OVER = 23500,
        PACKET_DROPPED,
        LAST_BASE_MAC_CONTROL_KIND
    };

    bool stats, trace;
    unsigned int RREQIDCounter;
    unsigned int nodeSeqNo;
    bool localRepair;

    //VARS
    typedef std::map<LAddress::L3Type,int> addressSeqNoMap_t;
    typedef std::set<LAddress::L3Type> addressSet_t;

    //last known addresses seqNo
    addressSeqNoMap_t nodesLastKnownSeqNoMap;

    //route map
    int routeMapElementMaxLifetime;
    typedef struct routeMapElement
    {
        LAddress::L3Type destAddr;
        int destSeqNo;
        LAddress::L3Type nextHop;
        int hopCount;
        simtime_t lifeTime;
        addressSet_t* precursors;
    } routeMapElement_t;
    typedef std::map<LAddress::L3Type,routeMapElement_t*> routeMap_t;
    typedef std::set<routeMapElement_t*> routeSet_t;
    routeMap_t routeMap;

    //RREQ vector
    unsigned int RREQVectorMaxSize;
    int RREQVectorMaintenancePeriod;
    int RREQVectorElementMaxLifetime;
    struct RREQVectorElement
    {
        int     RREQ_ID;
        LAddress::L3Type srcAddr;
        simtime_t lifeTime;
    };
    std::vector<RREQVectorElement*> RREQVector;

    // packet map
    int packetMapMaxPktQueueElementLifetime;
    int packetMapMaintenancePeriod;
    struct pktQueueElement
    {
        LAddress::L3Type    destAddr;
        simtime_t   lifeTime;
        ApplPkt*    packet;
    };
    typedef std::queue<pktQueueElement*> pktQueue_t;
    typedef std::map<LAddress::L3Type,pktQueue_t> pktMap_t;
    pktMap_t pktMap;


    //METHODS
    void handleUpperMsg(cMessage *);
    void handleLowerMsg(cMessage *);
    void handleSelfMsg(cMessage *);
    void handleLowerControl(cMessage *);
    void handleUpperControl(cMessage *);

    void handleUpperApplPkt(cMessage*);
    void handleLowerRREQ(cMessage*);
    void handleLowerRREP(cMessage*);
    void handleLowerRERR(cMessage*);
    void handleLowerDATA(cMessage*);
    void handleLowerControlPacketDropped (cMessage*);
    void handleLowerControlTXOver(cMessage*);
    void sendRERRByRemovedRoutes(routeSet_t*);

    //Known nodes seq number
    int getNodeSeqNo(LAddress::L3Type);
    void upsertNodeSeqNo(LAddress::L3Type,int);

    //Routes Map
    routeMapElement_t* getRouteForDestination(LAddress::L3Type);
    void resetRouteLifeTime(LAddress::L3Type);
    bool hasRouteForDestination(LAddress::L3Type);
    void routeTableMaintenance();
    bool upsertReverseRoute(AODVRouteRequest*);
    bool upsertForwardRoute(AODVRouteResponse*);
    bool upsertRoute(routeMapElement*);
    routeSet_t* removeRoutesByNextHop(LAddress::L3Type);
    routeSet_t* removeRoutesByDestinationAndNextHop(LAddress::L3Type, LAddress::L3Type);

    //RREQ vector
    bool hasRREQ(AODVRouteRequest*);
    void saveRREQ(AODVRouteRequest*);
    void runRREQSetMaintenance();

    //Application Packets map
    void destroyPktMap();
    void addToPktMap(ApplPkt *, LAddress::L3Type);
    ApplPkt* getFromPktMap(LAddress::L3Type);
    void runPktMapMaintenance();
};

#endif // AODV_ROUTE_H
