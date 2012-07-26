#include "MobileNodeAppLayerHoHuT.h"

using std::make_pair;

Define_Module(MobileNodeAppLayerHoHuT);

void MobileNodeAppLayerHoHuT::initialize(int stage)
{
    BaseModule::initialize(stage);

    if (stage == 0)
    {
        rssiValSignalId = registerSignal("rssiVal");

        dataOut = findGate("lowerGateOut");
        dataIn = findGate("lowerGateIn");
        ctrlOut = findGate("lowerControlOut");
        ctrlIn = findGate("lowerControlIn");

        nodeAddr = LAddress::L3Type(par("nodeAddr").longValue());
        debug = par("debug").boolValue();
        dataCollectionMode = par("dataCollectionMode").boolValue();
    }
    else if (stage == 1)
    {

    }
}

MobileNodeAppLayerHoHuT::~MobileNodeAppLayerHoHuT() {}

void MobileNodeAppLayerHoHuT::finish() {}

void MobileNodeAppLayerHoHuT::handleMessage(cMessage * msg)
{
	switch(msg->getKind())
	{
		case STATIC_NODE_SIGNATURE:
		    staticNodeSignature = check_and_cast<HoHuTApplPkt*>(msg);

		    emit(rssiValSignalId, staticNodeSignature->getSignalStrength());

			EV << "MobileNode says: Recebi uma STATIC_NODE_SIGNATURE" << endl;
			EV << "rssi=" << staticNodeSignature->getSignalStrength() << endl;
			EV << "src_address=" << staticNodeSignature->getSrcAddr() << endl;
			staticNodesRSSITable.insert(make_pair(staticNodeSignature->getSrcAddr(),staticNodeSignature->getSignalStrength()));
			EV << "samples available for node: " << staticNodesRSSITable.count(staticNodeSignature->getSrcAddr()) << endl;

			if (dataCollectionMode)
			{
			    //write position, beaconID and RSSI in table
			}
			else
			{
                EV << "MobileNode says: Sending response to static node:" << staticNodeSignature->getSrcAddr() << endl;
                mobileNoteRSSIMean = new HoHuTApplPkt("resp-sig",MOBILE_NODE_RSSI_MEAN);
                mobileNoteRSSIMean->setDestAddr(staticNodeSignature->getSrcAddr());
                mobileNoteRSSIMean->setSrcAddr(nodeAddr);
                mobileNoteRSSIMean->setSignalStrength(staticNodeSignature->getSignalStrength()); //por agora manda o último valor recebido
                NetwControlInfo::setControlInfo(mobileNoteRSSIMean,mobileNoteRSSIMean->getDestAddr());
                send(mobileNoteRSSIMean,dataOut);
			}
			delete msg;
			break;
		default:
		    error("Unknown msg kind. MsgKind="+msg->getKind());
		    break;
	}
}


