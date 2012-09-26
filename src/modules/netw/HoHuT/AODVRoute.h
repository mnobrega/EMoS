#include <omnetpp.h>
#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"

class HoHuTApplPkt;
class WiseRoutePkt;

class MIXIM_API AODVRoute : public BaseNetwLayer
{
public:
    virtual void initialize(int);
    virtual void finish();

protected:
    enum messagesTypes
    {
        DATA,
        RREQ,
        RREP,
        RERR
    };

    bool stats, trace;

    //stats stuff
    int totalSend;
    int totalRreqSend;
    int totalRreqRec;
    int totalRrepSend;
    int totalRrepRec;
    int totalRrepAckSend;
    int totalRrepAckRec;
    int totalRerrSend;
    int totalRerrRec;

    virtual void handleUpperMsg(cMessage* msg);
    virtual void handleLowerMsg(cMessage* msg);
    virtual void handleSelfMsg(cMessage* msg) { };
    virtual void handleLowerControl(cMessage* msg){ delete msg; }
    virtual void handleUpperControl(cMessage* msg) { delete msg; }
    WiseRoutePkt* encapsMsg(HoHuTApplPkt *pkt);
    HoHuTApplPkt* decapsMsg(WiseRoutePkt *pkt);

    //route table
    typedef struct routeTableEntry
    {
        LAddress::L3Type nextHop;
        int numHops;
    };
    typedef std::map<LAddress::L3Type,routeTableEntry> tRouteTable;
    tRouteTable routeTable;
    virtual void updateRouteTable(const tRouteTable::key_type& destAddress, const LAddress::L3Type& nextHop, int numHops);
};
