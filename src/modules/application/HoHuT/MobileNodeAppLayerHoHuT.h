#ifndef MOBILE_NODE_APP_LAYER_HOHUT_H
#define MOBILE_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include "BaseModule.h"
#include "BaseMobility.h"
#include "NetwControlInfo.h"

#include "HoHuTApplPkt_m.h"

class MobileNodeAppLayerHoHuT : public BaseModule
{
    public:
		virtual ~MobileNodeAppLayerHoHuT();
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *msg);
		virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
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

        // position signal tracking
        static const simsignalwrap_t mobilityStateChangedSignal;
        Coord currentPosition;

    private:
        // messages
        HoHuTApplPkt* staticNodeSignature;
        HoHuTApplPkt* mobileNoteRSSIMean;

        //statistic
        simsignal_t rssiValSignalId;

        // structure to store received RSSIs
        typedef std::multimap<int,float> tStaticNodesRSSITable;
        tStaticNodesRSSITable staticNodesRSSITable;
};

#endif // MOBILE_NODE_APP_LAYER_H
