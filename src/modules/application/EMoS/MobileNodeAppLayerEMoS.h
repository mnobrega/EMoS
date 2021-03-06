/* -*- mode:c++ -*- ********************************************************
 * file:        MobileNodeAppLayer.h
 *
 * author:      Márcio Nóbrega
 *
 * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
 *
 *              Implements the mobile node application layer in EMoS. This
 *              node has the hability to build the radioMap XML file during
 *              the offline phase. In the online phase it collects all the
 *              static nodes signatures during a pre-configure time period
 *              and sends it to the base station for position estimation.
 *              It has also the ability to send messages to any node in the
 *              WSN.
 *
 ***************************************************************************
 * part of:     EMoS - Elder Monitoring System
 ***************************************************************************/

#ifndef MOBILE_NODE_APP_LAYER_EMOS_H
#define MOBILE_NODE_APP_LAYER_EMOS_H

#include <omnetpp.h>
#include <queue>
#include <map>
#include <vector>
#include "BaseApplLayer.h"
#include "BaseMobility.h"
#include "NetwControlInfo.h"
#include "NetwToApplControlInfo.h"
#include "EMoSApplPkt_m.h"
#include "ApplPkt_m.h"
#include "Coord.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "string.h"
#include "algorithm"
#include "MiXiMDefs.h"
#include "FindModule.h"
#include "BaseNetwLayer.h"
#include "AddressingInterface.h"
#include "unistd.h"


class MobileNodeAppLayerEMoS : public BaseApplLayer
{
    public:
		virtual ~MobileNodeAppLayerEMoS();
		virtual void initialize(int stage);
		virtual void finish();

        enum MSG_TYPES
        {
            STATIC_NODE_SIG,         //broadcast-ttl1 - static node signature
            BASE_STATION_SIG,        //broadcast-fwd - base station signature (inform the mobile nodes of the base station netwaddr)
            STATIC_NODE_MSG,         //unicast - static node msg (hoven or other monitoring house devices)
            MOBILE_NODE_MSG          //unicast - mobile node msg (alarm, posTrackingReq, etc)
        };
        //identifies the type of EMoSAppl msg
        enum EMoS_MSG_TYPE
        {
            COLLECTED_RSSI,         //msg with the means of several rssis collected from the static nodes
            HELP_REQUEST,           //msg with a help request sent by the mobile node
            HUMAN_TEMPERATURE_MEAN, //human temperature
            HUMAN_HEART_BPM_MEAN    //human heart BPM mean
        };
        enum SELF_MSG_TYPES
        {
            SEND_COLLECTED_DATA_TIMER   //self msg - send the static nodes collected data
        };

    protected:
        bool debug;
        bool stats;

        LAddress::L3Type myAppAddr;
        double firstPosX;
        double firstPosY;

        const static simsignalwrap_t mobilityStateChangedSignal;

        typedef std::vector<LAddress::L3Type> addressVec_t;
        typedef std::map<LAddress::L3Type,double> addressRSSIMap_t;
        typedef std::multimap<LAddress::L3Type,double> addressRSSIMultiMap_t;

        // position signal tracking
        Coord currentPosition;
        Coord previousPosition;
        simsignal_t rssiValSignalId;

        // timers
        cMessage* selfTimer;

        // structure to store received RSSIs
        addressVec_t staticNodeAddrCollected;
        addressRSSIMultiMap_t staticNodesRSSITable;
        int collectedDataSendingTimePeriod;

        //radio map
        bool calibrationMode;
        unsigned int minimumStaticNodesForSample;
        unsigned int maxPositionPDFsSize;
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
        void sendCollectedDataToClosestStaticNode();
        void sendMobileNodeMsg(char*,LAddress::L3Type);
        bool hasCollectedNode(LAddress::L3Type);

        //radio map
        xmlDocPtr convertRadioMapToXML();
        xmlDocPtr convertRadioMapClustersToXML();
        void clusterizeRadioMapPosition(radioMapPosition_t*);
        coordVec_t* getCoordSetByClusterKey(addressVec_t*);
        bool hasCollectedPosition(Coord pos);

        //mobility
        virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

        //aux
        double convertTodBm(double valueInWatts);
        const xmlChar* convertNumberToXMLString(double number);


        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }
};

#endif // MOBILE_NODE_APP_LAYER_H
