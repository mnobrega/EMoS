#ifndef BASE_STATION_APP_LAYER_EMOS_H
#define BASE_STATION_APP_LAYER_EMOS_H

#include <omnetpp.h>
#include <queue>
#include <map>
#include <vector>
#include "BaseApplLayer.h"
#include "NetwControlInfo.h"
#include "NetwToApplControlInfo.h"
#include "EMoSApplPkt_m.h"
#include "ApplPkt_m.h"
#include "Coord.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <math.h>

class BaseStationAppLayerEMoS : public BaseApplLayer
{
    public:
		virtual ~BaseStationAppLayerEMoS();
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

    protected:
        bool debug;
        bool useClustering;
        bool useCenterOfMass;
        bool usePhySpaceTimeAvg;
        cXMLElement* radioMapXML;
        cXMLElement* radioMapClustersXML;
        cXMLElement* normalStandardDistribXML;

        typedef std::vector<LAddress::L3Type> addressVec_t;

        //PHYSICAL SPACE TIME AVG
        unsigned int phySpaceTimeAvgWindowSize;
        typedef std::queue<Coord> coordQueue_t;
        typedef std::map<LAddress::L3Type,coordQueue_t*> nodeLocationsMap_t;
        nodeLocationsMap_t estimatedNodesLocationsMap;

        //CENTER OF MASS
        unsigned int maxCenterOfMassPositions;
        typedef struct coordProbability
        {
            Coord pos;
            double probability;
        }coordProbability_t;
        struct coordProbabilityCompare
        {
            bool operator() (coordProbability_t* lhs, coordProbability_t* rhs)
            {
                return lhs->probability>rhs->probability; //probability DESC
            }
        };
        typedef std::set<coordProbability_t*,coordProbabilityCompare> coordProbSet_t;

        //STAT RESULTS
        bool recordStats;
        typedef std::map<LAddress::L3Type,cOutVector*> mobileNodesPosErrors_t;
        mobileNodesPosErrors_t mobileNodesPosErrors;

        //STANDARD NORMAL DISTRIB
        typedef std::map<double,double> normalStandardTable_t;
        normalStandardTable_t normalStandardTable;

        //RADIO MAP
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
        unsigned int clusterKeySize;

        //CLUSTERS
        typedef std::vector<Coord> coordVec_t;
        typedef std::map<addressVec_t*,coordVec_t*> radioMapCluster_t;
        radioMapCluster_t radioMapClusters;

        //COLLECTED NODES
        unsigned int maxPositionPDFsSize;
        typedef struct staticNodeRSSISample
        {
            LAddress::L3Type addr;
            double mean;
        }staticNodeRSSISample_t;
        struct staticNodeRSSISampleCompare
        {
            bool operator() (staticNodeRSSISample_t* lhs, staticNodeRSSISample_t* rhs)
            {
                return lhs->mean>rhs->mean; //mean DESC
            }
        };
        typedef std::set<staticNodeRSSISample*,staticNodeRSSISampleCompare> staticNodeSamplesSet_t;
        typedef std::map<LAddress::L3Type,double> addressRSSIMap_t;

        //METHODS
		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);
        void handleMobileNodeMsg (cMessage *);

        void loadNormalStandard(cXMLElement*);
        void loadRadioMapFromXML(cXMLElement*);
        void loadRadioMapClustersFromXML(cXMLElement*);

        //position calculation
        Coord getNodeLocation(staticNodeSamplesSet_t*);
        radioMapSet_t* getCandidatePositions(staticNodeSamplesSet_t*);
        coordVec_t* getCoordSetByClusterKey(addressVec_t* clusterKey);
        void insertIntoNodeLocationsQueue(LAddress::L3Type,Coord);
        Coord getTimeNodeAveragedLocation(LAddress::L3Type);

        //STATS
        void recordMobileNodePosError(LAddress::L3Type, Coord, Coord);

        //AUX
        double convertStringToNumber(const std::string&);
        staticNodeSamplesSet_t* getOrderedCollectedRSSIs(addressRSSIMap_t*);
        double roundNumber(double, int);
        double getDistance(Coord,Coord);

        //not implemented
        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }
};

#endif // BASE_STATION_APP_LAYER_H
