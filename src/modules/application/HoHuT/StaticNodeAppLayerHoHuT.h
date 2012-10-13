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
            STATIC_NODE_SIG,         //broadcast-ttl1 - static node signature
            BASE_STATION_SIG,        //broadcast-fwd - base station signature (inform the mobile nodes of the base station netwaddr)
            STATIC_NODE_MSG,         //unicast - static node msg (hoven or other monitoring house devices)
            MOBILE_NODE_MSG          //unicast - mobile node msg (alarm, posTrackingReq, etc)
        };
        //identifies the type of HoHuTAppl msg
        enum HOHUT_MSG_TYPE
        {
            COLLECTED_RSSI,         //msg with the means of several rssis collected from the static nodes
            HELP_REQUEST,           //msg with a help request sent by the mobile node
            HUMAN_TEMPERATURE_MEAN, //human temperature
            HUMAN_HEART_BPM_MEAN    //human heart BPM mean
        };
		enum SELF_MSG_TYPES
		{
		    STATIC_NODE_SIG_TIMER,   //self msg -signature sending timer
		    STATIC_NODE_MSG_TIMER   // self msg - msg s
		};

    protected:
        bool debug;

        // CONSTANTS
        //node signature
        int nodeSigStartingTime;
        int nodeSigPeriod;
        bool beaconMode;
        LAddress::L3Type baseStationNetwAddr;

        // timers
        cMessage* selfTimer;

        //METHODS
		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);
        void handleMobileNodeMsg(cMessage*);
        void sendStaticNodeSig();
        void sendStaticNodeMsg(char*,LAddress::L3Type);

        //not implemented
        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }
};

#endif // STATIC_NODE_APP_LAYER_H
