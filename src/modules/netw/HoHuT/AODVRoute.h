#include <omnetpp.h>
#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"

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
        DATA,
        CHECK_PKT_QUEUE_TIMER
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
    void handleLowerControl(cMessage * msg){ delete msg; }
    void handleUpperControl(cMessage * msg) { delete msg; }
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
    bool hasRouteForDestination(LAddress::L3Type);
    void checkRouteTable();


    // packets queue
    int pktQueueElementLifetime;
    int pktQueueCheckingPeriod;
    struct pktQueueElement
    {
        LAddress::L3Type    destAddr;
        simtime_t   lifeTime;
        NetwPkt*    packet;
    };
    std::vector<pktQueueElement*> pktQueue;
    void destroyPktQueue();
    void addToPktQueue(NetwPkt *);
    void checkPktQueue();
};
