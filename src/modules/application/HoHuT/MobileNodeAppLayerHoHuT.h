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
#include "MiXiMDefs.h"


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

    protected:
        bool debug;
        bool stats;
        bool calibrationMode;
        unsigned int minimumStaticNodesForSample;
        int clusterKeySize;
        const static simsignalwrap_t mobilityStateChangedSignal;

        // position signal tracking
        Coord currentPosition;
        Coord previousPosition;
        simsignal_t rssiValSignalId;

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

        //METHODS
		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);

        //static node sigs handling
        virtual void handleLowerStaticNodeSig (cMessage *);
        virtual double getStaticNodeMeanRSSI(LAddress::L3Type);
        void sendMobileNodeMsg(char*,LAddress::L3Type);

        //radio map
        virtual xmlDocPtr getRadioMapClustered();
        virtual xmlNodePtr getStaticNodePDFXMLNode(LAddress::L3Type);

        //stats
        virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

        //aux
        virtual double convertTodBm(double valueInWatts);
        virtual const char* convertNumberToString(double number);
        virtual bool inArray(const int needle,const std::vector<int> haystack);

        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }
};

#endif // MOBILE_NODE_APP_LAYER_H
