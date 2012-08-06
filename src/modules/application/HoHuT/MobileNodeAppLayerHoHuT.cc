#include "MobileNodeAppLayerHoHuT.h"
#include "Coord.h"

Define_Module(MobileNodeAppLayerHoHuT);

const simsignalwrap_t MobileNodeAppLayerHoHuT::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void MobileNodeAppLayerHoHuT::initialize(int stage)
{
    BaseModule::initialize(stage);

    if (stage == 0) //read config params
    {
        dataOut = findGate("lowerGateOut");
        dataIn = findGate("lowerGateIn");
        ctrlOut = findGate("lowerControlOut");
        ctrlIn = findGate("lowerControlIn");
        debug = par("debug").boolValue();
        stats = par("stats").boolValue();
        calibrationMode = par("calibrationMode").boolValue();
        staticNodeSigsCollectingLimit = par("staticNodeSigsCollectingLimit");
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
        rssiValSignalId = registerSignal("rssiVal");
        findHost()->subscribe(mobilityStateChangedSignal, this);
        previousPosition = new Coord(0,0,0);
        dataCollectingActive = true;
    }
}

MobileNodeAppLayerHoHuT::~MobileNodeAppLayerHoHuT() {}

void MobileNodeAppLayerHoHuT::finish() {}

void MobileNodeAppLayerHoHuT::handleMessage(cMessage * msg)
{
    switch(msg->getKind())
    {
        case STATIC_NODE_SIGNATURE:
            this->handleStaticNodeSig(msg);
            delete msg;
            break;
        default:
            error("Unknown msg kind. MsgKind="+msg->getKind());
            break;
    }
}

void MobileNodeAppLayerHoHuT::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal)
    {
        BaseMobility* baseMobility;
        baseMobility = dynamic_cast<BaseMobility *>(obj);
        currentPosition = baseMobility->getCurrentPosition();
    }
}

float MobileNodeAppLayerHoHuT::getStaticNodeSigMean(LAddress::L3Type staticNodeAddress)
{
    float auxSum = 0;
    std::pair<std::multimap<int,float>::iterator, std::multimap<int,float>::iterator> ppp;
    ppp = staticNodesRSSITable.equal_range(staticNodeAddress);
    for (std::multimap<int,float>::iterator it=ppp.first; it != ppp.second; ++it)
    {
        auxSum += (*it).second; //rssi value
    }
    return auxSum/staticNodesRSSITable.count(staticNodeAddress);
}


void MobileNodeAppLayerHoHuT::handleStaticNodeSig(cMessage * msg)
{
    float meanForStaticNode=0;
    staticNodeSignature = check_and_cast<HoHuTApplPkt*>(msg);

    EV << "MobileNode says: Recebi uma STATIC_NODE_SIGNATURE" << endl;
    EV << "rssi=" << staticNodeSignature->getSignalStrength() << endl;
    EV << "src_address=" << staticNodeSignature->getSrcAddr() << endl;

    if (stats)
    {
        emit(rssiValSignalId, staticNodeSignature->getSignalStrength());
    }

    if (dataCollectingActive)
    {
        staticNodesRSSITable.insert(std::make_pair(staticNodeSignature->getSrcAddr(),staticNodeSignature->getSignalStrength()));
        EV << "samples available for node: " << staticNodesRSSITable.count(staticNodeSignature->getSrcAddr()) << endl;
    }

    if (calibrationMode)
    {
        EV << "X=" << currentPosition.x << " Y=" << currentPosition.y << endl;
        //if position changed calculate power mean and save (x,y,P)k for the node k in the correspondent file
        //- check if new position already collected. error if already collected.
        if (previousPosition != currentPosition)
        {
            dataCollectingActive = false;

            if (staticNodesRSSITable.size()>=staticNodeSigsCollectingLimit)
            {
                std::multimap<int,float>::iterator it_nodeRSSI;
                std::map<int,cXMLElement*>::const_iterator it_nodeXML;
                cXMLElement* rssiSamplesXML;

                EV << "Position has changed and collected enough static nodes!!! Write list of received rssis to files" << endl;
                //for each static node collected
                for (std::multimap<int,float>::iterator it_nodeRSSI = staticNodesRSSITable.begin(), end=staticNodesRSSITable.end(); it_nodeRSSI!= end; it_nodeRSSI=staticNodesRSSITable.upper_bound(it_nodeRSSI->first))
                {
                    meanForStaticNode = this->getStaticNodeSigMean(it_nodeRSSI->first);
                    EV << "MobileNode says: (X=" << previousPosition.x << ", Y=" << previousPosition.y << ") rssiMean=" << meanForStaticNode << endl;

                    it_nodeXML = staticNodesRSSIXMLs.find(it_nodeRSSI->first);
                    if (it_nodeXML==staticNodesRSSIXMLs.end())
                    {
                        EV << "nao ha XML. tenho de criar um novo" << endl;
                        staticNodesRSSIXMLs.insert(std::pair<int,cXMLElement*>(it_nodeRSSI->first,rssiSamplesXML));
                    }
                    else
                    {
                        EV << "ha XML. utilizar o que já existe" << endl;
                    }
                }
                staticNodesRSSITable.clear();
            }
            else
            {
                EV << "Not enough static node sigs collected for the current position." << endl;
                staticNodesRSSITable.clear();
            }
            previousPosition = currentPosition;
        }
        //if position unchanged keep collecting the (staticNodeAddress,rssi) pairs
        else
        {
            dataCollectingActive = true;
            EV << "Position has NOT changed!!! Keep collecting static node sigs";
        }
    }
    else
    {
        if (staticNodesRSSITable.count(staticNodeSignature->getSrcAddr())>=staticNodeSigsCollectingLimit)
        {
            dataCollectingActive = false;
            EV << "MobileNode says: Sending response to static node:" << staticNodeSignature->getSrcAddr() << endl;
            mobileNodeRSSIMean = new HoHuTApplPkt("resp-sig",MOBILE_NODE_RSSI_MEAN);
            mobileNodeRSSIMean->setSignalStrength(this->getStaticNodeSigMean(staticNodeSignature->getSrcAddr()));
            mobileNodeRSSIMean->setDestAddr(staticNodeSignature->getSrcAddr());
            mobileNodeRSSIMean->setSrcAddr(-1);
            NetwControlInfo::setControlInfo(mobileNodeRSSIMean,mobileNodeRSSIMean->getDestAddr());
            send(mobileNodeRSSIMean,dataOut);
            staticNodesRSSITable.erase(staticNodeSignature->getSrcAddr());
            dataCollectingActive = true;
        }
    }

}
