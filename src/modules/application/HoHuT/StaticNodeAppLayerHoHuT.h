#ifndef STATIC_NODE_APP_LAYER_HOHUT_H
#define STATIC_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include <queue>
#include <map>
#include "BaseApplLayer.h"
#include "NetwControlInfo.h"
#include "NetwToApplControlInfo.h"
#include "HoHuTApplPkt_m.h"
#include "ApplPkt_m.h"

class StaticNodeAppLayerHoHuT : public BaseApplLayer
{
    public:
		virtual ~StaticNodeAppLayerHoHuT();
		virtual void initialize(int stage);
		virtual void finish();

		enum MSG_TYPES
        {
            STATIC_NODE_SIG,         //broadcast - static node signature
            STATIC_NODE_MSG,         //unicast - static node msg (hoven or other monitoring house devices)
            MOBILE_NODE_MSG          //unicast - mobile node msg (alarm, posTrackingReq, etc)
        };
		enum SELF_MSG_TYPES
		{
		    STATIC_NODE_SIG_TIMER   //self msg -signature sending timer
		};
		enum AODV_CTRL_MSG_TYPES
        {
		    HAS_ROUTE,
		    HAS_ROUTE_ACK,
		    DELIVERY_ACK,
		    DELIVERY_ERROR,
		    DELIVERY_LOCAL_REPAIR
        };

    protected:
        bool debug;

        // CONSTANTS
        //node signature
        int nodeSigStartingTime;
        int nodeSigPeriod;
        //packet map
        int packetMapMaxPktQueueElementLifetime;
        int packetMapMaintenancePeriod;


        // VARS
        // timers
        cMessage* selfTimer;

        // packet map
        struct pktQueueElement
        {
            LAddress::L3Type    destAddr;
            simtime_t   lifeTime;
            HoHuTApplPkt*    packet;
        };
        typedef std::queue<pktQueueElement*> pktQueue_t;
        typedef std::map<LAddress::L3Type,pktQueue_t> pktMap_t;
        pktMap_t pktMap;


        //METHODS
		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);
        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }

        void sendStaticNodeSig();
        void sendStaticNodeMsg(char*,LAddress::L3Type);

        void destroyPktMap();
        void addToPktMap(HoHuTApplPkt *);
        HoHuTApplPkt* getFromPktMap(LAddress::L3Type);
        void runPktMapMaintenance();
};

#endif // STATIC_NODE_APP_LAYER_H
