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

        if (calibrationMode)
        {
            radioMapXML = xmlNewDoc(BAD_CAST "1.0");
            radioMapXMLRoot = xmlNewNode(NULL,BAD_CAST "radioMap");
            xmlDocSetRootElement(radioMapXML, radioMapXMLRoot);
        }
    }
}

MobileNodeAppLayerHoHuT::~MobileNodeAppLayerHoHuT() {}

void MobileNodeAppLayerHoHuT::finish()
{
    if(calibrationMode)
    {
        xmlSaveFormatFileEnc("xml_radio_maps/radioMap.xml",radioMapXML,"UTF-8",1);
        //xmlSaveFormatFileEnc("xml_radio_maps/radioMapClustered.xml",this->getRadioMapClustered(),"UTF-8",1);
        xmlFreeDoc(radioMapXML);
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
        xmlNodePtr positionNode = NULL;
        xmlNodePtr staticNodePDFXML = NULL;

        //for each static node collected
        for (unsigned int i=0; i<staticNodeAddressesDetected.size();i++)
        {
            if (staticNodesRSSITable.count(staticNodeAddressesDetected[i])>=minimumStaticNodesForSample)
            {
                if (positionNode==NULL)
                {
                    positionNode = xmlNewNode(NULL, BAD_CAST "position");
                    xmlNewProp(positionNode, BAD_CAST "x", BAD_CAST this->convertNumberToString(previousPosition.x));
                    xmlNewProp(positionNode, BAD_CAST "y", BAD_CAST this->convertNumberToString(previousPosition.y));
                }
                staticNodePDFXML = this->getStaticNodePDFXMLNode(staticNodeAddressesDetected[i]);
                xmlAddChild(positionNode, staticNodePDFXML);
            }
        }
        if (positionNode!=NULL)
        {
            xmlAddChild(radioMapXMLRoot, positionNode);
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
    if (!inArray(srcAddress,staticNodeAddressesDetected))
    {
        staticNodeAddressesDetected.push_back(srcAddress);
    }
    if (stats)
    {
       emit(rssiValSignalId, srcRSSI);
    }

    delete msg;
    debugEV << "samples available for node: " << staticNodesRSSITable.count(srcAddress) << endl;
}
xmlDocPtr MobileNodeAppLayerHoHuT::getRadioMapClustered()
{
    std::map<double, int> pairsMeanStaticNodeAddress;
    std::multimap<clusterKey,xmlNodePtr> clusteredRadioMap;
    clusterKey clusterKey;
    double mean;
    int staticNodeAddress;

    xmlNodePtr position = NULL;
    xmlNodePtr staticNodePDF = NULL;
    xmlDocPtr radioMapClusteredXML = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr radioMapClusteredXMLRoot = xmlNewNode(NULL,BAD_CAST "radioMapClustered");
    xmlDocSetRootElement(radioMapClusteredXML, radioMapClusteredXMLRoot);

    for (position=radioMapXMLRoot->children; position; position=position->next)
    {
        for (staticNodePDF=position->children; staticNodePDF; staticNodePDF=staticNodePDF->next)
        {
            mean = atof(reinterpret_cast<const char*>(xmlGetProp(staticNodePDF, BAD_CAST "mean")));
            staticNodeAddress = atoi(reinterpret_cast<const char*>(xmlGetProp(staticNodePDF,BAD_CAST "address")));
            pairsMeanStaticNodeAddress.insert(std::make_pair(mean,staticNodeAddress));
        }

        //TODO: sorting map<double,int>

        //TODO: sorting by address the clusterkeys

        //TODO: save pair(clusterkey, xmlnodeptr) in the clusteredRadioMap
    }

    //TODO: circle thru the clusteredRadioMap and put the result in the XML file

    return radioMapClusteredXML;
}
double MobileNodeAppLayerHoHuT::getStaticNodeMeanRSSI(LAddress::L3Type staticNodeAddress)
{
    cStdDev stat("staticNodeStat");

    std::pair<std::multimap<int,double>::iterator, std::multimap<int,double>::iterator> ppp;
    ppp = staticNodesRSSITable.equal_range(staticNodeAddress);
    for (std::multimap<int,double>::iterator it=ppp.first; it != ppp.second; ++it)
    {
        stat.collect((*it).second);
    }
    return stat.getMean();
}
xmlNodePtr MobileNodeAppLayerHoHuT::getStaticNodePDFXMLNode(LAddress::L3Type staticNodeAddress)
{
    cStdDev stat("staticNodeStat");
    std::multimap<int,double>::iterator it;

    xmlNodePtr staticNodePDFXMLNode = xmlNewNode(NULL,BAD_CAST "staticNodePDF");
    xmlNewProp(staticNodePDFXMLNode,BAD_CAST "address", BAD_CAST this->convertNumberToString(staticNodeAddress));

    std::pair<std::multimap<int,double>::iterator, std::multimap<int,double>::iterator> ppp;
    ppp = staticNodesRSSITable.equal_range(staticNodeAddress);
    for (it=ppp.first; it != ppp.second; ++it)
    {
        stat.collect((*it).second);
    }

    xmlNewProp(staticNodePDFXMLNode,BAD_CAST "mean", BAD_CAST this->convertNumberToString(this->convertTodBm(stat.getMean())));
    xmlNewProp(staticNodePDFXMLNode,BAD_CAST "stdDev", BAD_CAST this->convertNumberToString(this->convertTodBm(stat.getStddev())));

    return staticNodePDFXMLNode;
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

bool MobileNodeAppLayerHoHuT::inArray(const int needle,const std::vector<int> haystack)
{
    int max = haystack.size();
    if (max==0) return false;
    for (int i=0; i<max; i++)
    {
        if (haystack[i]==needle)
        {
            return true;
        }
    }
    return false;
}
