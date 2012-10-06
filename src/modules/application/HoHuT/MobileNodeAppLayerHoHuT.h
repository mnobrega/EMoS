#ifndef MOBILE_NODE_APP_LAYER_HOHUT_H
#define MOBILE_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include <queue>
#include <map>
#include <vector>
#include "BaseApplLayer.h"
#include "BaseMobility.h"
#include "NetwControlInfo.h"
#include "NetwToApplControlInfo.h"
#include "HoHuTApplPkt_m.h"
#include "ApplPkt_m.h"
#include "Coord.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "string.h"
#include "algorithm"


class MobileNodeAppLayerHoHuT : public BaseApplLayer
{
    public:
		virtual ~MobileNodeAppLayerHoHuT();
		virtual void initialize(int stage);
		virtual void finish();

        enum MSG_TYPES
        {
            STATIC_NODE_SIG,         //broadcast - static node signature
            STATIC_NODE_MSG,         //unicast - static node msg (hoven or other monitoring house devices)
            MOBILE_NODE_MSG          //unicast - mobile node msg (alarm, posTrackingReq, etc)
        };
		enum AODV_CTRL_MSG_TYPES
        {
		    HAS_ROUTE,
		    HAS_ROUTE_ACK,
		    DELIVERY_ACK,
		    DELIVERY_ERROR,
		    DELIVERY_LOCAL_REPAIR
        };

	//	static const simsignalwrap_t mobilityStateChangedSignal;

    protected:
        bool debug;
        bool stats;

        // CONSTANTS
        //packet map
        int packetMapMaxPktQueueElementLifetime;
        int packetMapMaintenancePeriod;
        bool calibrationMode;
        int minimumStaticNodesForSample;
        int clusterKeySize;

        // position signal tracking

        Coord currentPosition;
        Coord previousPosition;
        simsignal_t rssiValSignalId;

        // VARS
        // timers
        cMessage* selfTimer;

        // structure to store received RSSIs
        std::vector<int> staticNodeAddressesDetected;
        std::multimap<int,double> staticNodesRSSITable;

        //radio map clustering
        typedef std::vector<int> clusterKey;

        // calibrationMode - radio Map XML
        xmlDocPtr radioMapXML;
        xmlNodePtr radioMapXMLRoot;

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

        //static node sigs handling
        virtual void handleLowerStaticNodeSig (cMessage *msg);
        virtual double getStaticNodeMeanRSSI(LAddress::L3Type staticNodeAddress);

        //radio map
        virtual xmlDocPtr getRadioMapClustered();
        virtual xmlNodePtr getStaticNodePDFXMLNode(LAddress::L3Type staticNodeAddress);

        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
        void sendMobileNodeMsg(char*,LAddress::L3Type);

        //packets map
        void destroyPktMap();
        void addToPktMap(HoHuTApplPkt *);
        HoHuTApplPkt* getFromPktMap(LAddress::L3Type);
        void runPktMapMaintenance();

        //aux
        virtual double convertTodBm(double valueInWatts);
        virtual const char* convertNumberToString(double number);
        virtual bool inArray(const int needle,const std::vector<int> haystack);
};

#endif // MOBILE_NODE_APP_LAYER_H
