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
        xmlSaveFormatFileEnc("xml_radio_maps/radioMap.xml",convertRadioMapToXML(),"UTF-8",1);
        clusterizeRadioMap();
        xmlSaveFormatFileEnc("xml_radio_maps/radioMapClustered.xml",convertClusteredRadioMapToXML(),"UTF-8",1);
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

        radioMapPosition_t* radioMapPosition=NULL;
        for (unsigned int i=0; i<staticNodeAddrCollected.size();i++)
        {
            if (staticNodesRSSITable.count(staticNodeAddrCollected[i])>=minimumStaticNodesForSample)
            {
                ppp = staticNodesRSSITable.equal_range(staticNodeAddrCollected[i]);
                for (it=ppp.first; it != ppp.second; ++it)
                {
                    stat.collect((*it).second);
                }

                staticNodePDF_t* staticNodePDF = (struct staticNodePDF*) malloc(sizeof(struct staticNodePDF));
                staticNodePDF->addr = staticNodeAddrCollected[i];
                staticNodePDF->mean = convertTodBm(stat.getMean());
                staticNodePDF->stdDev = convertTodBm(stat.getStddev());

                if (radioMapPosition==NULL)
                {
                    radioMapPosition = (struct radioMapPosition*) malloc(sizeof(struct radioMapPosition));
                    radioMapPosition->pos = previousPosition;
                    radioMapPosition->staticNodesPDFSet = new staticNodesPDFSet_t;
                    radioMapPosition->staticNodesPDFSet->insert(staticNodePDF);
                }
                else
                {
                    radioMapPosition->staticNodesPDFSet->insert(staticNodePDF);
                }
            }
        }
        if (radioMapPosition!=NULL)
        {
            radioMap.insert(radioMapPosition);
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
void MobileNodeAppLayerHoHuT::clusterizeRadioMap()
{
    radioMapSet_t::iterator it;
    staticNodesPDFSet_t::iterator it2;
    clusteredRadioMap_t::iterator it3;

    staticNodesPDFSet_t* staticNodes;
    radioMapPosition_t* position;
    addressVec_t* clusterKey, cRadioMapClusterKeyVec;
    radioMapSet_t* radioMapSet;
    LAddress::L3Type nodeAddr;
    unsigned int i;

    for (it=radioMap.begin(); it!=radioMap.end();it++)
    {
        position = (*it);
        staticNodes = position->staticNodesPDFSet;
        clusterKey = new addressVec_t;
        for (it2=staticNodes->begin(),i=0 ; it2!=staticNodes->end() && i<clusterKeySize ; it2++)
        {
            nodeAddr = (*it2)->addr;
            clusterKey->push_back(nodeAddr);
            i++;
        }
        if (clusterKey->size()==clusterKeySize)
        {
            radioMapSet = getRadioMapSetByClusterKey(clusterKey);
            if (radioMapSet!=NULL)
            {
                //already created.
                radioMapSet->insert(position);
            }
            else
            {
                radioMapSet = new radioMapSet_t;
                radioMapSet->insert(position);
                clusteredRadioMap.insert(std::pair<addressVec_t*,radioMapSet_t*>(clusterKey,radioMapSet));
            }
        }
        else
        {
            debugEV << "Clusterkey is not valid for position " << convertNumberToString(position->pos.x) << "," << convertNumberToString(position->pos.y) << endl;
            error ("Invalid clusterKey detected");
        }
    }

}
MobileNodeAppLayerHoHuT::radioMapSet_t* MobileNodeAppLayerHoHuT::getRadioMapSetByClusterKey(addressVec_t* addressVec)
{
    //TODO
}
xmlDocPtr MobileNodeAppLayerHoHuT::convertRadioMapToXML()
{
    radioMapSet_t::iterator it;
    staticNodesPDFSet_t::iterator it2;
    radioMapPosition_t* position;
    staticNodePDF_t* sig;
    staticNodesPDFSet_t* staticNodes;
    xmlNodePtr positionNode;
    xmlNodePtr staticNodePDFXMLNode;

    xmlDocPtr xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr xmlDocRoot = xmlNewNode(NULL,BAD_CAST "radioMap");
    xmlDocSetRootElement(xmlDoc, xmlDocRoot);

    for (it=radioMap.begin(); it!=radioMap.end();it++)
    {
        position = (*it);
        positionNode = xmlNewNode(NULL, BAD_CAST "position");
        xmlNewProp(positionNode, BAD_CAST "x", BAD_CAST convertNumberToString(position->pos.x));
        xmlNewProp(positionNode, BAD_CAST "y", BAD_CAST convertNumberToString(position->pos.y));

        staticNodes = position->staticNodesPDFSet;
        for (it2=staticNodes->begin(); it2!=staticNodes->end();it2++)
        {
            sig = (*it2);
            staticNodePDFXMLNode = xmlNewNode(NULL,BAD_CAST "staticNodePDF");
            xmlNewProp(staticNodePDFXMLNode,BAD_CAST "address", BAD_CAST convertNumberToString(sig->addr));
            xmlNewProp(staticNodePDFXMLNode,BAD_CAST "mean", BAD_CAST convertNumberToString(sig->mean));
            xmlNewProp(staticNodePDFXMLNode,BAD_CAST "stdDev", BAD_CAST convertNumberToString(sig->stdDev));
            xmlAddChild(positionNode,staticNodePDFXMLNode);
        }
        xmlAddChild(xmlDocRoot,positionNode);
    }

    return xmlDoc;
}
xmlDocPtr MobileNodeAppLayerHoHuT::convertClusteredRadioMapToXML()
{
    clusteredRadioMap_t::iterator it;
    radioMapSet_t::iterator it2;
    staticNodesPDFSet_t::iterator it3;
    addressVec_t::iterator it4;

    addressVec_t* clusterKey;
    radioMapSet_t* positionsPtr;
    radioMapSet_t positions;
    radioMapPosition_t* position;
    staticNodePDF_t* sig;
    staticNodesPDFSet_t* staticNodes;

    xmlNodePtr clusterNode;
    xmlNodePtr clusterKeyNode;
    xmlNodePtr clusterKeyStaticNode;
    xmlNodePtr positionNode;
    xmlNodePtr staticNodePDFXMLNode;

    LAddress::L3Type staticNodeAddress;

    xmlDocPtr xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr xmlDocRoot = xmlNewNode(NULL,BAD_CAST "clusteredRadioMap");
    xmlDocSetRootElement(xmlDoc, xmlDocRoot);

    for(it=clusteredRadioMap.begin();it!=clusteredRadioMap.end();it++)
    {
        clusterKey = it->first;
        clusterNode = xmlNewNode(NULL, BAD_CAST "cluster");
        clusterKeyNode = xmlNewNode(NULL, BAD_CAST "clusterKey");
        for (it4=clusterKey->begin(); it4!=clusterKey->end();it4++)
        {
            staticNodeAddress = *it4;
            clusterKeyStaticNode = xmlNewNode(NULL, BAD_CAST "staticNode");
            xmlNewProp(clusterKeyStaticNode, BAD_CAST "address", BAD_CAST convertNumberToString(staticNodeAddress));
            xmlAddChild(clusterKeyNode,clusterKeyStaticNode);
        }
        xmlAddChild(clusterNode,clusterKeyNode);

        positionsPtr = it->second;
        positions = (*positionsPtr);
//        for (it2=positions.begin(); it2!=positions.end; it2++)
//        {
//            position = (*it2);
//            positionNode = xmlNewNode(NULL, BAD_CAST "position");
//            xmlNewProp(positionNode, BAD_CAST "x", BAD_CAST convertNumberToString(position->pos.x));
//            xmlNewProp(positionNode, BAD_CAST "y", BAD_CAST convertNumberToString(position->pos.y));
//
//            staticNodes = position->staticNodesPDFSet;
//            for (it3=staticNodes->begin(); it3!=staticNodes->end();it3++)
//            {
//                sig = (*it3);
//                staticNodePDFXMLNode = xmlNewNode(NULL,BAD_CAST "staticNodePDF");
//                xmlNewProp(staticNodePDFXMLNode,BAD_CAST "address", BAD_CAST convertNumberToString(sig->addr));
//                xmlNewProp(staticNodePDFXMLNode,BAD_CAST "mean", BAD_CAST convertNumberToString(sig->mean));
//                xmlNewProp(staticNodePDFXMLNode,BAD_CAST "stdDev", BAD_CAST convertNumberToString(sig->stdDev));
//                xmlAddChild(positionNode,staticNodePDFXMLNode);
//            }
//            xmlAddChild(clusterNode,positionNode);
//        }

        xmlAddChild(xmlDocRoot,clusterNode);
    }
    return xmlDoc;
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
