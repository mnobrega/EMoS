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

void MobileNodeAppLayerHoHuT::handleStaticNodeSig(cMessage * msg)
{
    HoHuTApplPkt* staticNodeSignature = check_and_cast<HoHuTApplPkt*>(msg);

    EV << "MobileNode says: Received STATIC_NODE_SIGNATURE from node: " << staticNodeSignature->getSrcAddr() << endl;

    //calibrationMode and new position
    if (calibrationMode && previousPosition != currentPosition)
    {
        xmlNodePtr positionNode = NULL;
        xmlNodePtr staticNodePDFXML = NULL;
        bool addPositionToRadioMap = false;

        //for each static node collected
        for (unsigned int i=0; i<staticNodeAddressesDetected.size();i++)
        {
            if (staticNodesRSSITable.count(staticNodeAddressesDetected[i])>=minimumStaticNodesForSample)
            {
                addPositionToRadioMap = true;
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

        if (addPositionToRadioMap)
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
    //staticNodesRSSITable.insert(std::make_pair(staticNodeSignature->getSrcAddr(),staticNodeSignature->getSignalStrength()));
    if (!inArray(staticNodeSignature->getSrcAddr(),staticNodeAddressesDetected))
    {
        staticNodeAddressesDetected.push_back(staticNodeSignature->getSrcAddr());
    }
    if (stats)
    {
       //emit(rssiValSignalId, staticNodeSignature->getSignalStrength());
    }

    EV << "samples available for node: " << staticNodesRSSITable.count(staticNodeSignature->getSrcAddr()) << endl;
}



/*** AUX functions TODO remove this to another class common to all HoHuT app layers***/
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


