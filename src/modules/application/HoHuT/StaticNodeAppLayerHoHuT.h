#ifndef STATIC_NODE_APP_LAYER_HOHUT_H
#define STATIC_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
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
		    DELIVERY_ACK
        };

    protected:
        bool debug;

		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);
        virtual void sendStaticNodeSig();
        virtual void sendStaticNodeMsg(const char* msgData);
        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }

    private:
        // constants
        int INITIAL_DELAY;
        int BEACON_INTERVAL;
        int PAYLOAD_SIZE;

        // timers
        cMessage* selfTimer;

        // packets queue
        int pktQueueElementLifetime;
        int pktQueueCheckingPeriod;
        struct pktQueueElement
        {
            LAddress::L3Type    destAddr;
            simtime_t   lifeTime;
            HoHuTApplPkt*    packet;
        };
        std::vector<pktQueueElement*> pktQueue;
        void destroyPktQueue();
        void addToPktQueue(HoHuTApplPkt *);
        void checkPktQueue();
};

#endif // STATIC_NODE_APP_LAYER_H
