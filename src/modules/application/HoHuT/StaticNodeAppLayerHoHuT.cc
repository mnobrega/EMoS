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
    	selfTimer = new cMessage("beacon-timer",STATIC_NODE_SIG_TIMER);
    	scheduleAt(simTime() + INITIAL_DELAY +uniform(0,0.001), selfTimer);
    }
}

StaticNodeAppLayerHoHuT::~StaticNodeAppLayerHoHuT() {}

void StaticNodeAppLayerHoHuT::finish() {}

void StaticNodeAppLayerHoHuT::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
        case STATIC_NODE_SIG_TIMER:
            //sendStaticNodeMsg();
            if (this->getParentModule()->getIndex()==0)
            {
                sendStaticNodeMsg("test");
            }
            delete (msg);
//            if (!selfTimer->isScheduled())
//            {
//                scheduleAt(simTime()+BEACON_INTERVAL,selfTimer);
//            }
            break;
        default:
            error("Unknown msg kind. MsgKind="+msg->getKind());
            break;
    }
}

void StaticNodeAppLayerHoHuT::handleLowerMsg(cMessage * msg)
{
    HoHuTApplPkt* m = static_cast<HoHuTApplPkt*>(msg);
    switch (msg->getKind())
    {
        case STATIC_NODE_MSG:
            debugEV << "Received a node msg from node: " << m->getSrcAddr() << endl;
            break;
        case STATIC_NODE_SIG:
            debugEV << "Received a node msg from node: " << m->getSrcAddr() << endl;
            break;
        default:
            error("Unknown msg type received.");
            break;
    }
}

void StaticNodeAppLayerHoHuT::handleLowerControl(cMessage* msg)
{
    //TODO - this should be used for failed msg deliveries by the netwlayer due to not founding a route in acceptable time
}

/**** APP ****/

void StaticNodeAppLayerHoHuT::sendStaticNodeSig()
{
    debugEV << "Sending SIGNATURE" << endl;
    HoHuTApplPkt* nodeSignature = new HoHuTApplPkt("node-sig",STATIC_NODE_SIG);
    nodeSignature->setSignalStrength(-1);
    nodeSignature->setSrcAddr(-1);
    nodeSignature->setDestAddr(LAddress::L3BROADCAST);
    NetwControlInfo::setControlInfo(nodeSignature, nodeSignature->getDestAddr());
    send(nodeSignature, dataOut);
}

void StaticNodeAppLayerHoHuT::sendStaticNodeMsg(const char* msgData)
{
    debugEV << "Sending static node msg" << endl;
    HoHuTApplPkt* aodvTestDataMsg = new HoHuTApplPkt("aodv-test",STATIC_NODE_MSG);
    aodvTestDataMsg->setData(msgData);
    NetwControlInfo::setControlInfo(aodvTestDataMsg, 1003);
    send(aodvTestDataMsg, dataOut);
}
