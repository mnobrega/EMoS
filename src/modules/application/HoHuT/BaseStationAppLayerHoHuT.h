#ifndef BASE_STATION_APP_LAYER_HOHUT_H
#define BASE_STATION_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include <queue>
#include <map>
#include <vector>
#include "BaseApplLayer.h"
#include "NetwControlInfo.h"
#include "NetwToApplControlInfo.h"
#include "HoHuTApplPkt_m.h"
#include "ApplPkt_m.h"
#include "Coord.h"
#include <sstream>

class BaseStationAppLayerHoHuT : public BaseApplLayer
{
    public:
		virtual ~BaseStationAppLayerHoHuT();
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

    protected:
        bool debug;
        bool useClustering;
        cXMLElement* radioMapXML;
        cXMLElement* radioMapClustersXML;

        typedef std::vector<LAddress::L3Type> addressVec_t;

        //radio map
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
        typedef std::vector<Coord> coordVec_t;
        typedef std::map<addressVec_t*,coordVec_t*> radioMapCluster_t;
        radioMapCluster_t radioMapClusters;

        //METHODS
		virtual void handleSelfMsg(cMessage *);
        virtual void handleLowerMsg(cMessage *);
        virtual void handleLowerControl(cMessage *);

        void loadRadioMapFromXML(cXMLElement*);
        void loadRadioMapClustersFromXML(cXMLElement*);

        //not implemented
        virtual void handleUpperMsg(cMessage * m) { delete m; }
        virtual void handleUpperControl(cMessage * m) { delete m; }

        double convertStringToNumber(const std::string&);
};

#endif // BASE_STATION_APP_LAYER_H
