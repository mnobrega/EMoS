#ifndef MOBILE_NODE_APP_LAYER_HOHUT_H
#define MOBILE_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include "BaseModule.h"
#include "BaseMobility.h"
#include "NetwControlInfo.h"

//c++ libs used
#include "Coord.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "string.h"
#include "algorithm"
#include <vector>
#include <map>

#include "src/modules/messages/HoHuT/HoHuTApplPkt_m.h"

class MobileNodeAppLayerHoHuT : public BaseModule
{
    public:
		virtual ~MobileNodeAppLayerHoHuT();
		virtual xmlDocPtr getRadioMapClustered();
		virtual void initialize(int stage);
		virtual void handleStaticNodeSig (cMessage *msg);
		virtual xmlNodePtr getStaticNodePDFXMLNode(LAddress::L3Type staticNodeAddress);
		virtual double getStaticNodeMeanRSSI(LAddress::L3Type staticNodeAddress);
		virtual void handleMessage(cMessage *msg);
		virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
		virtual void finish();

		virtual double convertTodBm(double valueInWatts);
        virtual const char* convertNumberToString(double number);
        virtual bool inArray(const int needle,const std::vector<int> haystack);

		enum APPl_MSG_TYPES
		{
			SELF_TIMER,
			STATIC_NODE_SIGNATURE,
			MOBILE_NODE_RSSI_MEAN
		};

    protected:
        // gates
    	int dataOut;
    	int dataIn;
        int ctrlOut;
        int ctrlIn;

        // module parameters
        bool debug;
        bool stats;
        unsigned int minimumStaticNodesForSample;
        bool calibrationMode;       //mode in which the node receives static_refs and then writes files with the means for each (x,y)

        // position signal tracking
        static const simsignalwrap_t mobilityStateChangedSignal;
        Coord currentPosition;
        Coord previousPosition;

    private:
        // messages
        HoHuTApplPkt* staticNodeSignature;
        HoHuTApplPkt* mobileNodeRSSIMean;

        //statistic
        simsignal_t rssiValSignalId;

        // structure to store received RSSIs
        std::vector<int> staticNodeAddressesDetected;
        std::multimap<int,double> staticNodesRSSITable;

        //radio map clustering
        int clusterKeySize;
        typedef std::vector<int> clusterKey;
        typedef std::multimap<clusterKey,xmlNodePtr> pairClusterKeyXMLNodePtr;

        // calibrationMode - radio Map XML
        xmlDocPtr radioMapXML;
        xmlNodePtr radioMapXMLRoot;
};

#endif // MOBILE_NODE_APP_LAYER_H
