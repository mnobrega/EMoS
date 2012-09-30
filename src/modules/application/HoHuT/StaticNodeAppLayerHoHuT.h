#ifndef STATIC_NODE_APP_LAYER_HOHUT_H
#define STATIC_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include <queue>
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
            STATIC_NODE_SIG,        //broadcast - static node signature
            MOBILE_NODE_MSG,        //unicast - mobile node msg (alarm, posTrackingReq, etc)
            STATIC_NODE_MSG,        //unicast - static node msg (hoven or other monitoring house devices)
            STATIC_NODE_SIG_TIMER   //self msg -signature sending timer
        };
		enum AODV_CTRL_MSG_TYPES
        {
		    HAS_ROUTE,
		    HAS_ROUTE_ACK,
		    DELIVERY_ACK,
		    DELIVERY_ERROR
        };

    protected:
        bool debug;
        // constants
        int INITIAL_DELAY;
        int BEACON_INTERVAL;
        int PAYLOAD_SIZE;

        // timers
        cMessage* selfTimer;

        // packets queue
        int maxPktQueueElementLifetime;
        int pktQueueMaintenancePeriod;
        int maxPacketDeliveryTries;
        int sentPktQueueTriesCounter;
        HoHuTApplPkt* sentPktQueueBuffer;
        struct pktQueueElement
        {
            LAddress::L3Type    destAddr;
            simtime_t   lifeTime;
            HoHuTApplPkt*    packet;
        };
        std::queue<pktQueueElement*> pktQueue;

		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);
        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }

        void sendStaticNodeSig();
        void sendStaticNodeMsg(const char* msgData);

        void destroyPktQueue();
        void addToPktQueue(HoHuTApplPkt *);
        HoHuTApplPkt* getFromPktQueue();
        void runPktQueueMaintenance();

};

#endif // STATIC_NODE_APP_LAYER_H
