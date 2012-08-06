#include "StaticNodeAppLayerHoHuT.h"

Define_Module(StaticNodeAppLayerHoHuT);

void StaticNodeAppLayerHoHuT::initialize(int stage)
{
    BaseModule::initialize(stage);
    if (stage == 0) //read config params
    {
    	debugEV<< "in initialize() stage 0...";
        dataOut = findGate("lowerGateOut");
        dataIn = findGate("lowerGateIn");
        ctrlOut = findGate("lowerControlOut");
        ctrlIn = findGate("lowerControlIn");
        debug = par("debug").boolValue();
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";
    	INITIAL_DELAY = 5;
    	BEACON_INTERVAL = 1;
    	selfTimer = new cMessage("beacon-timer",SELF_TIMER);
    	scheduleAt(simTime() + INITIAL_DELAY +uniform(0,0.001), selfTimer);
    }
}

StaticNodeAppLayerHoHuT::~StaticNodeAppLayerHoHuT() {}

void StaticNodeAppLayerHoHuT::finish() {}

void StaticNodeAppLayerHoHuT::handleMessage(cMessage * msg)
{
	switch(msg->getKind())
	{
		case SELF_TIMER:
		    EV << "Sending SIGNATURE";
			nodeSignature = new HoHuTApplPkt("node-sig",STATIC_NODE_SIGNATURE);
			nodeSignature->setSignalStrength(-1);
			nodeSignature->setSrcAddr(-1);
			nodeSignature->setDestAddr(LAddress::L3BROADCAST);
			NetwControlInfo::setControlInfo(nodeSignature, nodeSignature->getDestAddr());
			send(nodeSignature, dataOut);
			if (!selfTimer->isScheduled())
			{
				scheduleAt(simTime()+BEACON_INTERVAL,selfTimer);
			}
			break;
		case STATIC_NODE_SIGNATURE:
			delete msg;  //not for me. im a static node
			break;
		case MOBILE_NODE_RSSI_MEAN:
		    EV << "WEE! Recebi uma media dos meus node-sigs" << endl;
		    mobileNodeStaticSigMean = check_and_cast<HoHuTApplPkt*>(msg);
		    EV << "MobileNode says: Recebi uma STATIC_NODE_SIGNATURE" << endl;
		    EV << "rssi=" << mobileNodeStaticSigMean->getSignalStrength() << endl;
		    EV << "src_address=" << mobileNodeStaticSigMean->getSrcAddr() << endl;
		    delete msg;
		    break;
		default:
		    error("Unknown msg kind. MsgKind="+msg->getKind());
			break;
	}
}
