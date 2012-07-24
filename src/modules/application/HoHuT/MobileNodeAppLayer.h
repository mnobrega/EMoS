#ifndef MOBILE_NODE_APP_LAYER_H
#define MOBILE_NODE_APP_LAYER_H

#include <omnetpp.h>
#include "BaseModule.h"
#include "NetwControlInfo.h"
#include "HoHuTApplPkt_m.h"

class MobileNodeAppLayer : public BaseModule
{
    public:
		virtual ~MobileNodeAppLayer();
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *msg);
		virtual void finish();

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
        LAddress::L3Type nodeAddr;
        bool debug;
        bool dataCollectionMode;

    private:
        // messages
        HoHuTApplPkt* staticNodeSignature;
        HoHuTApplPkt* mobileNoteRSSIMean;

        //statistic
        simsignal_t rssiValSignal;

        // structure to store received RSSIs
        typedef std::multimap<int,float> tStaticNodesRSSITable;
        tStaticNodesRSSITable staticNodesRSSITable;
};

#endif // MOBILE_NODE_APP_LAYER_H
