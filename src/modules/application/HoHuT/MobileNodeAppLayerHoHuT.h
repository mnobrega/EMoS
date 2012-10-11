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

        const static simsignalwrap_t mobilityStateChangedSignal;

        typedef std::vector<LAddress::L3Type> addressVec_t;
        typedef std::multimap<LAddress::L3Type,double> addressRSSIMap_t;

        // position signal tracking
        Coord currentPosition;
        Coord previousPosition;
        simsignal_t rssiValSignalId;

        // timers
        cMessage* selfTimer;

        // structure to store received RSSIs
        addressVec_t staticNodeAddrCollected;
        addressRSSIMap_t staticNodesRSSITable;

        //radio map
        bool calibrationMode;
        unsigned int minimumStaticNodesForSample;
        typedef struct staticNodePDF
        {
            LAddress::L3Type addr;
            double mean;
            double stdDev;
        }staticNodePDF_t;
        struct staticNodePDFCompare
        {
            bool operator() (staticNodePDF_t* lhs, staticNodePDF_t* rhs)
            {
                return lhs->mean>rhs->mean; //mean DESC - staticNodesPDF are ordered from largest mean to lowest
            }
        };
        typedef std::set<staticNodePDF_t*,staticNodePDFCompare> staticNodesPDFSet_t;
        typedef struct radioMapPosition
        {
            Coord pos;
            staticNodesPDFSet_t* staticNodesPDFSet;
        }radioMapPosition_t;
        typedef std::set<radioMapPosition_t*> radioMapSet_t;
        radioMapSet_t radioMap;


        //clusters - radio map
        unsigned int clusterKeySize;
        typedef std::vector<Coord> coordVec_t;
        typedef std::map<addressVec_t*,coordVec_t*> radioMapCluster_t;
        radioMapCluster_t radioMapClusters;


        //METHODS
		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);

        //static node sigs handling
        void handleLowerStaticNodeSig (cMessage *);
        void sendMobileNodeMsg(char*,LAddress::L3Type);
        bool hasCollectedNode(LAddress::L3Type);

        //radio map
        xmlDocPtr convertRadioMapToXML();
        xmlDocPtr convertRadioMapClustersToXML();
        void clusterizeRadioMapPosition(radioMapPosition_t*);
        coordVec_t* getCoordSetByClusterKey(addressVec_t*);

        //mobility
        virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

        //aux
        virtual double convertTodBm(double valueInWatts);
        virtual const char* convertNumberToString(double number);


        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }
};

#endif // MOBILE_NODE_APP_LAYER_H
