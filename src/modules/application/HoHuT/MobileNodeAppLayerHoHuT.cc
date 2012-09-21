#include "MobileNodeAppLayerHoHuT.h"

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
        minimumStaticNodesForSample = par("minimumStaticNodesForSample");
        clusterKeySize = par("clusterKeySize");
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
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
        xmlSaveFormatFileEnc("xml_radio_maps/radioMapClustered.xml",this->getRadioMapClustered(),"UTF-8",1);
        xmlFreeDoc(radioMapXML);
        xmlCleanupParser();
        xmlMemoryDump();
    }
}

xmlDocPtr MobileNodeAppLayerHoHuT::getRadioMapClustered()
{
    std::vector<clusterKey> coordinateKeys;
    clusterKey staticNodePDFKey;
    xmlNodePtr coordinate = NULL;
    xmlNodePtr staticNodePDF = NULL;

    xmlDocPtr radioMapClusteredXML = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr radioMapClusteredXMLRoot = xmlNewNode(NULL,BAD_CAST "radioMapClustered");
    xmlDocSetRootElement(radioMapClusteredXML, radioMapClusteredXMLRoot);

    //TODO
    //circle radioMapXML and create clusters. in the end add them to radioMapClusteredXMLRoot
//    for (coordinate=radioMapXMLRoot->children; coordinate; coordinate=coordinate->next)
//    {
//        for (staticNodePDF=coordinate->children; staticNodePDF; staticNodePDF=staticNodePDF->next)
//        {
//            staticNodePDFKey.address = (int) xmlGetProp(staticNodePDF,BAD_CAST "address");
//            staticNodePDFKey.mean = (double) xmlGetProp(staticNodePDF,BAD_CAST "mean");
//            coordinateKeys.insert(staticNodePDFKey);
//        }
//
//        std::sort(coordinateKeys.begin(), coordinateKeys.end(), byMean());
//    }

    return radioMapClusteredXML;
}

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

    xmlNodePtr staticNodePDFXMLNode = xmlNewNode(NULL,BAD_CAST "staticNodePDF");
    xmlNewProp(staticNodePDFXMLNode,BAD_CAST "address", BAD_CAST this->convertToString(staticNodeAddress));

    std::pair<std::multimap<int,double>::iterator, std::multimap<int,double>::iterator> ppp;
    ppp = staticNodesRSSITable.equal_range(staticNodeAddress);
    for (std::multimap<int,double>::iterator it=ppp.first; it != ppp.second; ++it)
    {
        stat.collect((*it).second);
    }

    xmlNewProp(staticNodePDFXMLNode,BAD_CAST "mean", BAD_CAST this->convertToString(this->convertTodBm(stat.getMean())));
    xmlNewProp(staticNodePDFXMLNode,BAD_CAST "stdDev", BAD_CAST this->convertToString(this->convertTodBm(stat.getStddev())));

    return staticNodePDFXMLNode;
}

double MobileNodeAppLayerHoHuT::convertTodBm(double valueInWatts)
{
    return 10*log10(valueInWatts/0.001);
}

const char* MobileNodeAppLayerHoHuT::convertToString(double value)
{
    std::ostringstream s;
    s << value;
    std::string output = s.str();
    const char* inCharPointerFormat = output.c_str();
    return inCharPointerFormat;
}

void MobileNodeAppLayerHoHuT::handleStaticNodeSig(cMessage * msg)
{
    staticNodeSignature = check_and_cast<HoHuTApplPkt*>(msg);

    EV << "MobileNode says: Received STATIC_NODE_SIGNATURE" << endl;
    EV << "rssi=" << staticNodeSignature->getSignalStrength() << endl;
    EV << "src_address=" << staticNodeSignature->getSrcAddr() << endl;

    if (stats)
    {
        emit(rssiValSignalId, staticNodeSignature->getSignalStrength());
    }

    //calibrationMode and new position
    if (calibrationMode && previousPosition != currentPosition)
    {
        EV << "Position has changed! previousPosition:" << previousPosition.x << previousPosition.y << " currentPosition:" << currentPosition.x << currentPosition.y << endl;

        xmlNodePtr coordinateNode = NULL;
        bool addCoordinateToRadioMap = false;
        std::multimap<int,double>::iterator it_nodeRSSI;

        //for each static node collected
        for (std::multimap<int,double>::iterator it_nodeRSSI = staticNodesRSSITable.begin(), end=staticNodesRSSITable.end(); it_nodeRSSI!= end; it_nodeRSSI=staticNodesRSSITable.upper_bound(it_nodeRSSI->first))
        {
            if (staticNodesRSSITable.count(it_nodeRSSI->first)>=minimumStaticNodesForSample)
            {
                addCoordinateToRadioMap = true;
                if (coordinateNode==NULL)
                {
                    coordinateNode = xmlNewNode(NULL, BAD_CAST "coordinate");
                    xmlNewProp(coordinateNode, BAD_CAST "x", BAD_CAST this->convertToString(previousPosition.x));
                    xmlNewProp(coordinateNode, BAD_CAST "y", BAD_CAST this->convertToString(previousPosition.y));
                }
                xmlAddChild(coordinateNode, this->getStaticNodePDFXMLNode(it_nodeRSSI->first));
                EV << "MobileNode says: (X=" << previousPosition.x << ", Y=" << previousPosition.y << ") nodeId:" << it_nodeRSSI->first << " rssiMean=" << 0 << endl;
            }
            else
            {
                EV << "Not enough static node signatures collected for the current position for the node " << it_nodeRSSI->first << endl;
            }
        }

        if (addCoordinateToRadioMap)
        {
            xmlAddChild(radioMapXMLRoot, coordinateNode);
        }

        staticNodesRSSITable.clear();
        previousPosition = currentPosition;
    }
    else if (!calibrationMode) //not calibration mode
    {
        if (staticNodesRSSITable.count(staticNodeSignature->getSrcAddr())>=minimumStaticNodesForSample)
        {
            EV << "MobileNode says: Sending response to static node:" << staticNodeSignature->getSrcAddr() << endl;
            mobileNodeRSSIMean = new HoHuTApplPkt("resp-sig",MOBILE_NODE_RSSI_MEAN);
            mobileNodeRSSIMean->setSignalStrength(this->getStaticNodeMeanRSSI(staticNodeSignature->getSrcAddr()));
            mobileNodeRSSIMean->setDestAddr(staticNodeSignature->getSrcAddr());
            mobileNodeRSSIMean->setSrcAddr(-1);
            NetwControlInfo::setControlInfo(mobileNodeRSSIMean,mobileNodeRSSIMean->getDestAddr());
            send(mobileNodeRSSIMean,dataOut);
            staticNodesRSSITable.erase(staticNodeSignature->getSrcAddr());
        }
    }

    //collect static nodes signatures
    staticNodesRSSITable.insert(std::make_pair(staticNodeSignature->getSrcAddr(),staticNodeSignature->getSignalStrength()));
    EV << "samples available for node: " << staticNodesRSSITable.count(staticNodeSignature->getSrcAddr()) << endl;
}
