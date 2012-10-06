#include "MobileNodeAppLayerHoHuT.h"

Define_Module(MobileNodeAppLayerHoHuT);

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

        packetMapMaxPktQueueElementLifetime = par("packetMapMaxPktQueueElementLifetime");
        packetMapMaintenancePeriod = par("packetMapMaintenancePeriod");

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
   //     findHost()->subscribe(mobilityStateChangedSignal, this);
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
    destroyPktMap();
    if(calibrationMode)
    {
        xmlSaveFormatFileEnc("xml_radio_maps/radioMap.xml",radioMapXML,"UTF-8",1);
        xmlSaveFormatFileEnc("xml_radio_maps/radioMapClustered.xml",this->getRadioMapClustered(),"UTF-8",1);
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
    switch (msg->getKind())
    {
        case STATIC_NODE_SIG:
            handleLowerStaticNodeSig(msg);
            break;
        case STATIC_NODE_MSG:
            debugEV << "Received a static node msg! " << endl;
            delete msg;
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void MobileNodeAppLayerHoHuT::handleLowerControl(cMessage* msg)
{
    HoHuTApplPkt* pkt;
    NetwToApplControlInfo* cInfo;
    ApplPkt* ctrlPkt = static_cast<ApplPkt*>(msg);

    switch (msg->getKind())
    {
        case HAS_ROUTE_ACK:
            pkt = getFromPktMap(ctrlPkt->getDestAddr());
            NetwControlInfo::setControlInfo(pkt, pkt->getNetwDestAddr());
            sendDown(pkt);
            delete msg;
            break;
        case DELIVERY_ACK:
            pkt = static_cast<HoHuTApplPkt*>(msg);
            cInfo = static_cast<NetwToApplControlInfo*>(msg->getControlInfo());
            debugEV << "Packet for destAddress: " << pkt->getNetwDestAddr() << " was delivered from: " << cInfo->getSrcNetwAddr() << endl;
            delete msg;
            break;
        case DELIVERY_ERROR:
            pkt = static_cast<HoHuTApplPkt*>(msg);
            cInfo = static_cast<NetwToApplControlInfo*>(msg->getControlInfo());
            debugEV << "Packet was not delivered to destAddress: " << pkt->getNetwDestAddr() << " from: " << cInfo->getSrcNetwAddr() << endl;
            bubble("PACKET LOST");
            delete msg;
            break;
        case DELIVERY_LOCAL_REPAIR:
            //TODO - add it to a pktMapLocalRepair buffer. Send new RREQ. When the RREQ arrives try again.
            pkt = static_cast<HoHuTApplPkt*>(msg);
            pkt->removeControlInfo();
            delete msg;
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}


/**** APP ****/
void MobileNodeAppLayerHoHuT::handleLowerStaticNodeSig(cMessage * msg)
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

    delete msg;
    EV << "samples available for node: " << staticNodesRSSITable.count(staticNodeSignature->getSrcAddr()) << endl;
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

void MobileNodeAppLayerHoHuT::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
//    if (signalID == mobilityStateChangedSignal)
//    {
//        BaseMobility* baseMobility;
//        baseMobility = dynamic_cast<BaseMobility *>(obj);
//        currentPosition = baseMobility->getCurrentPosition();
//    }
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

void MobileNodeAppLayerHoHuT::sendMobileNodeMsg(char* msgPayload, LAddress::L3Type netwDestAddr)
{
    debugEV << "Sending mobile node msg to netwDestAddr:" << netwDestAddr << endl;

    HoHuTApplPkt* appPkt = new HoHuTApplPkt("mobile-node-app-msg",MOBILE_NODE_MSG);
    appPkt->setPayload(msgPayload);
    appPkt->setNetwDestAddr(netwDestAddr);

    if (appPkt->getNetwDestAddr()==LAddress::L3BROADCAST)
    {
        debugEV << "It is broadcast. Send it immediately!" << endl;
        NetwControlInfo::setControlInfo(appPkt, appPkt->getNetwDestAddr());
        sendDown(appPkt);
    }
    else
    {
        debugEV << "It is not broadcast. Ask netw through the control channel if the route exists for node: " << appPkt->getNetwDestAddr() << endl;
        addToPktMap(appPkt);
        ApplPkt* ctrlPkt = new ApplPkt("ask-netw-for-route", HAS_ROUTE);
        ctrlPkt->setDestAddr(appPkt->getNetwDestAddr());
        sendControlDown(ctrlPkt);
    }
}

/// PACKET MAP
void MobileNodeAppLayerHoHuT::destroyPktMap()
{
    int count = 0;
    pktMap_t::iterator it;
    pktQueue_t pktQueue;

    for (it=pktMap.begin(); it!=pktMap.end(); it++)
    {
       while (it->second.size()>0)
       {
           pktQueueElement* qEl = it->second.front();
           it->second.pop();
           HoHuTApplPkt* pkt = static_cast<HoHuTApplPkt*>(qEl->packet);
           delete pkt;
           count++;
       }
       debugEV << count << " pkts were destroyed for destAddr:" << it->first << endl;
    }

}
void MobileNodeAppLayerHoHuT::addToPktMap(HoHuTApplPkt * pkt)
{

    debugEV << "Adding packet to map for address :" << pkt->getNetwDestAddr() << endl;
    pktQueueElement* qEl = (struct pktQueueElement *) malloc(sizeof(struct pktQueueElement));
    qEl->destAddr = pkt->getNetwDestAddr();
    qEl->lifeTime = simTime()+packetMapMaxPktQueueElementLifetime;
    qEl->packet = pkt;

    pktMap_t::iterator it = pktMap.find(pkt->getNetwDestAddr());
    if (it!=pktMap.end())
    {
        it->second.push(qEl);
    }
    else
    {
        pktQueue_t pktQueue;
        pktQueue.push(qEl);
        pktMap.insert(std::pair<LAddress::L3Type,pktQueue_t>(pkt->getNetwDestAddr(),pktQueue));
    }
}
HoHuTApplPkt* MobileNodeAppLayerHoHuT::getFromPktMap(LAddress::L3Type destAddr)
{
    pktMap_t::iterator it = pktMap.find(destAddr);
    if (it!=pktMap.end())
    {
        if (it->second.size()>0)
        {
            pktQueueElement* qEl = it->second.front();
            it->second.pop();
            HoHuTApplPkt* pkt = static_cast<HoHuTApplPkt*>(qEl->packet);
            if (it->second.size()==0)
            {
                pktMap.erase(it);
            }
            return pkt;
        }
    }
    return NULL;
}
void MobileNodeAppLayerHoHuT::runPktMapMaintenance()
{
    debugEV << "Checking packet queue for packets to send or expire!" << endl;
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
