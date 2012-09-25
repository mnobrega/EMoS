#ifndef STATIC_NODE_APP_LAYER_HOHUT_H
#define STATIC_NODE_APP_LAYER_HOHUT_H

#include <omnetpp.h>
#include "BaseModule.h"
#include "NetwControlInfo.h"

#include "src/modules/messages/HoHuT/HoHuTApplPkt_m.h"

class StaticNodeAppLayerHoHuT : public BaseModule
{
    public:
		virtual ~StaticNodeAppLayerHoHuT();
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage* msg);
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
        bool debug;

    private:
        // constants
        int INITIAL_DELAY;
        int BEACON_INTERVAL;
        int PAYLOAD_SIZE;

        // timers
        cMessage* selfTimer;
};

#endif // STATIC_NODE_APP_LAYER_H

