#include "MobileNodeAppLayerHoHuT.h"
#include "Coord.h"

using std::make_pair;

Define_Module(MobileNodeAppLayerHoHuT);

const simsignalwrap_t MobileNodeAppLayerHoHuT::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

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
        findHost()->subscribe(mobilityStateChangedSignal, this);
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

			//NOTE: there shouldnt be repetition of positions already collected
			if (dataCollectionMode)
			{
			    EV << "X=" << currentPosition.x << " Y=" << currentPosition.y << endl;
			    //if position unchanged keep collection the (staticNodeAddress,rssi) pairs

			    //else position changed calculate power mean and save (x,y,P)k for the node k in the correspondent file
			        //check if new position already collected. error if already collected.
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

void MobileNodeAppLayerHoHuT::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) {
    Enter_Method_Silent();
    BaseMobility* baseMobility;
    if (signalID == mobilityStateChangedSignal)
    {
        baseMobility = dynamic_cast<BaseMobility *>(obj);
        currentPosition = baseMobility->getCurrentPosition();
    }
}


