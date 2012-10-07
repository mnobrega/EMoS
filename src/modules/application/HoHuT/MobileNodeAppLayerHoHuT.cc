#include "MobileNodeAppLayerHoHuT.h"

Define_Module(MobileNodeAppLayerHoHuT);

const simsignalwrap_t MobileNodeAppLayerHoHuT::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void MobileNodeAppLayerHoHuT::initialize(int stage)
{
    BaseApplLayer::initialize(stage);

    if (stage == 0) //read config params
    {
    	debugEV<< "in initialize() stage 0...";
        lowerLayerOut = findGate("lowerLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerControlOut = findGate("lowerControlOut");
        lowerControlIn = findGate("lowerControlIn");

        debug = par("debug").boolValue();
        stats = par("stats").boolValue();
        calibrationMode = par("calibrationMode").boolValue();
        minimumStaticNodesForSample = par("minimumStaticNodesForSample");
        clusterKeySize = par("clusterKeySize");
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";

    	rssiValSignalId = registerSignal("rssiVal");
    	findHost()->subscribe(mobilityStateChangedSignal, this);
        previousPosition = new Coord(-1,-1,-1);
        currentPosition = new Coord(0,0,0);
    }
}

MobileNodeAppLayerHoHuT::~MobileNodeAppLayerHoHuT() {}

void MobileNodeAppLayerHoHuT::finish()
{
    if(calibrationMode)
    {
//        xmlSaveFormatFileEnc("xml_radio_maps/radioMap.xml",convertRadioMapToXML(),"UTF-8",1);
        //xmlSaveFormatFileEnc("xml_radio_maps/radioMapClustered.xml",this->getRadioMapClustered(),"UTF-8",1);
        xmlCleanupParser();
        xmlMemoryDump();
    }
}

//HANDLING MSGS
void MobileNodeAppLayerHoHuT::handleSelfMsg(cMessage * msg)
{
    delete msg;
}
void MobileNodeAppLayerHoHuT::handleLowerMsg(cMessage * msg)
{
    NetwToApplControlInfo* cInfo;
    HoHuTApplPkt* applPkt;

    switch (applPkt->getKind())
    {
        case STATIC_NODE_MSG:
            cInfo = static_cast<NetwToApplControlInfo*>(msg->removeControlInfo());
            applPkt = static_cast<HoHuTApplPkt*>(msg);
            debugEV << "received rssi: " << cInfo->getRSSI() << endl;
            debugEV << "Received a node msg from static node: " << cInfo->getSrcNetwAddr() << endl;
            debugEV << "msg data:" << applPkt->getPayload() << endl;
            delete msg;
            break;
        case MOBILE_NODE_MSG:
            cInfo = static_cast<NetwToApplControlInfo*>(msg->removeControlInfo());
            applPkt = static_cast<HoHuTApplPkt*>(msg);
            debugEV << "received rssi: " << cInfo->getRSSI() << endl;
            debugEV << "Received a node msg from mobile node: " << cInfo->getSrcNetwAddr() << endl;
            debugEV << "msg data:" << applPkt->getPayload() << endl;
            delete msg;
            break;
        case STATIC_NODE_SIG:
            handleLowerStaticNodeSig(msg);
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void MobileNodeAppLayerHoHuT::handleLowerControl(cMessage* msg)
{
    delete msg;
}


/**** APP ****/
void MobileNodeAppLayerHoHuT::handleLowerStaticNodeSig(cMessage * msg)
{
    HoHuTApplPkt* staticNodeSig = static_cast<HoHuTApplPkt*>(msg);

    NetwToApplControlInfo* cInfo = static_cast<NetwToApplControlInfo*>(staticNodeSig->removeControlInfo());
    LAddress::L3Type srcAddress = cInfo->getSrcNetwAddr();
    double srcRSSI = cInfo->getRSSI();

    debugEV << "MobileNode says: Received STATIC_NODE_SIGNATURE from node: " << srcAddress << endl;

    //calibrationMode and new position
    if (calibrationMode && previousPosition != currentPosition)
    {
        cStdDev stat("staticNodeStat");

        std::pair<addressRSSIMap_t::iterator,addressRSSIMap_t::iterator> ppp;
        addressRSSIMap_t::iterator it;

        staticNodePDF_t* staticNodePDF;
        staticNodesPDFSet_t* staticNodesPDFSet;

        for (unsigned int i=0; i<staticNodeAddrCollected.size();i++)
        {
            if (staticNodesRSSITable.count(staticNodeAddrCollected[i])>=minimumStaticNodesForSample)
            {
                ppp = staticNodesRSSITable.equal_range(staticNodeAddrCollected[i]);
                for (it=ppp.first; it != ppp.second; ++it)
                {
                    stat.collect((*it).second);
                }

                staticNodePDF = (struct staticNodePDF*) malloc(sizeof(struct staticNodePDF));
                staticNodePDF->addr = staticNodeAddrCollected[i];
                staticNodePDF->mean = convertTodBm(stat.getMean());
                staticNodePDF->stdDev = convertTodBm(stat.getStddev());

                if (radioMap.find(previousPosition)==radioMap.end())
                {
                    staticNodesPDFSet = new staticNodesPDFSet_t;
                    staticNodesPDFSet->insert(staticNodePDF);
                    radioMap.insert(std::pair<Coord,staticNodesPDFSet_t*>(previousPosition,staticNodesPDFSet));
                }
                else
                {
                    staticNodesPDFSet = radioMap.find(previousPosition)->second;
                    staticNodesPDFSet->insert(staticNodePDF);
                }
            }
        }
        staticNodesRSSITable.clear();
        previousPosition = currentPosition;
    }
    else if (!calibrationMode) //not calibration mode TODO - fix this in order to send a vector of measurements
    {
        //collect samples during t=x and when x is reached send the result to the closest static node
    }

    // COLLECT
    staticNodesRSSITable.insert(std::make_pair(srcAddress,srcRSSI));
    if (!hasCollectedNode(srcAddress))
    {
        staticNodeAddrCollected.push_back(srcAddress);
    }
    if (stats)
    {
       emit(rssiValSignalId, srcRSSI);
    }

    delete msg;
    debugEV << "samples available for node: " << staticNodesRSSITable.count(srcAddress) << endl;
}
bool MobileNodeAppLayerHoHuT::hasCollectedNode(LAddress::L3Type nodeAddr)
{
    for (unsigned int i=0; i<staticNodeAddrCollected.size();i++)
    {
        if (staticNodeAddrCollected[i]==nodeAddr)
        {
            return true;
        }
    }
    return false;
}
xmlDocPtr MobileNodeAppLayerHoHuT::convertRadioMapToXML()
{
//    radioMapSet_t::iterator it;
//    staticNodePDFSet_t::iterator it2;
//    radioMapPosition_t* pos;
//    staticNodePDF_t* sig;
//
//    xmlDocPtr xmlDoc = xmlNewDoc(BAD_CAST "1.0");
//    xmlNodePtr xmlDocRoot = xmlNewNode(NULL,BAD_CAST "radioMap");
//    xmlNodePtr positionNode;
//    xmlDocSetRootElement(xmlDoc, xmlDocRoot);
//
//    for (it=radioMapSet.begin(); it!=radioMapSet.end();it++)
//    {
//        pos = (*it);
//        positionNode = xmlNewNode(NULL, BAD_CAST "position");
//        xmlNewProp(positionNode, BAD_CAST "x", BAD_CAST this->convertNumberToString(pos->position.x));
//        xmlNewProp(positionNode, BAD_CAST "y", BAD_CAST this->convertNumberToString(pos->position.y));
//
//        staticNodePDFSet_t* staticNodes = pos->staticNodesPDF;
//        for (it2=staticNodes->begin(); it2!=staticNodes->end();it2++)
//        {
//            sig = (*it2);
//            xmlNodePtr staticNodePDFXMLNode = xmlNewNode(NULL,BAD_CAST "staticNodePDF");
//            xmlNewProp(staticNodePDFXMLNode,BAD_CAST "address", BAD_CAST this->convertNumberToString(sig->addr));
//            xmlNewProp(staticNodePDFXMLNode,BAD_CAST "mean", BAD_CAST this->convertNumberToString(sig->mean));
//            xmlNewProp(staticNodePDFXMLNode,BAD_CAST "stdDev", BAD_CAST this->convertNumberToString(sig->stdDev));
//            xmlAddChild(positionNode,staticNodePDFXMLNode);
//        }
//        xmlAddChild(xmlDocRoot,positionNode);
//    }
//
//    return xmlDoc;
}


// MOBILE NODE MSG
void MobileNodeAppLayerHoHuT::sendMobileNodeMsg(char* msgPayload, LAddress::L3Type netwDestAddr)
{
    debugEV << "Sending mobile node msg to netwDestAddr:" << netwDestAddr << endl;

    HoHuTApplPkt* appPkt = new HoHuTApplPkt("mobile-node-app-msg",MOBILE_NODE_MSG);
    appPkt->setPayload(msgPayload);
    NetwControlInfo::setControlInfo(appPkt, netwDestAddr);
    sendDown(appPkt);
}


// MOBILITY
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



// AUX
double MobileNodeAppLayerHoHuT::convertTodBm(double valueInWatts)
{
    return 10*log10(valueInWatts/0.001);
}

const char* MobileNodeAppLayerHoHuT::convertNumberToString(double number)
{
    std::ostringstream s;
    s << number;
    std::string output = s.str();
    const char* inCharPointerFormat = output.c_str();
    return inCharPointerFormat;
}
