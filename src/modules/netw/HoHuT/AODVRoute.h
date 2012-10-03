#ifndef AODV_ROUTE_H
#define AODV_ROUTE_H

#include <omnetpp.h>
#include <limits>
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
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
        DELIVERY_ERROR
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

    //CONSTANTS
    //RREQ vector
    bool localRepair;
    unsigned int RREQVectorMaxSize;
    int RREQVectorMaintenancePeriod;
    int RREQVectorElementMaxLifetime;
    //route map
    int routeMapElementMaxLifetime;

    //VARS
    typedef std::map<LAddress::L3Type,int> addressSeqNoMap_t;
    typedef std::set<LAddress::L3Type> addressSet_t;

    //last known addresses seqNo
    addressSeqNoMap_t nodesLastKnownSeqNoMap;

    //route map
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
    struct RREQVectorElement
    {
        int     RREQ_ID;
        LAddress::L3Type srcAddr;
        simtime_t lifeTime;
    };
    std::vector<RREQVectorElement*> RREQVector;


    //METHODS
    void handleUpperMsg(cMessage *);
    void handleLowerMsg(cMessage *);
    void handleSelfMsg(cMessage *);
    void handleLowerControl(cMessage * msg);
    void handleUpperControl(cMessage * msg);
    AODVData* encapsMsg(cPacket *);
    cPacket* decapsMsg(AODVData *);

    void handleUpperControlHasRoute(ApplPkt*);
    void handleLowerRREQ(cMessage*);
    void handleLowerRREP(cMessage*);
    void handleLowerDATA(cMessage*);

    int getNodeSeqNo(LAddress::L3Type);
    void upsertNodeSeqNo(LAddress::L3Type,int);

    routeMapElement_t* getRouteForDestination(LAddress::L3Type);
    bool hasRouteForDestination(LAddress::L3Type);
    void routeTableMaintenance();
    bool upsertReverseRoute(AODVRouteRequest*);
    bool upsertForwardRoute(AODVRouteResponse*);
    bool upsertRoute(routeMapElement*);
    routeSet_t removeRoutesByNextHop(LAddress::L3Type);

    bool hasRREQ(AODVRouteRequest*);
    void saveRREQ(AODVRouteRequest*);
    void runRREQSetMaintenance();

    const char* serializeAddressSeqNoMap(addressSeqNoMap_t);

};

#endif // AODV_ROUTE_H
