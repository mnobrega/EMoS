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
        collectedDataSendingTimePeriod = par("collectedDataSendingTimePeriod");
        maxPositionPDFsSize = par("maxPositionPDFsSize");

        firstPosX = par("firstPosX");
        firstPosY = par("firstPosY");

        if (maxPositionPDFsSize<clusterKeySize)
        {
            error("maxPositionPDFsSize has to be equal or greater than clusterKeySize. please check your config");
        }
    }
    else if (stage == 1) //initialize vars, subscribe signals, etc
    {
    	debugEV << "in initialize() stage 1...";

    	rssiValSignalId = registerSignal("rssiVal");
    	findHost()->subscribe(mobilityStateChangedSignal, this);

        previousPosition = new Coord(firstPosX,firstPosY,0);
        currentPosition = new Coord(firstPosX,firstPosY,0);

        if (!calibrationMode)
        {
            selfTimer = new cMessage("beacon-timer",SEND_COLLECTED_DATA_TIMER);
            scheduleAt(simTime() + collectedDataSendingTimePeriod, selfTimer);
        }

        cModule *const pHost = findHost();
        const cModule* netw  = FindModule<BaseNetwLayer*>::findSubModule(pHost);
        if(!netw) {
            netw = pHost->getSubmodule("netwl");
            if(!netw) {
                opp_error("Could not find network layer module. This means "
                          "either no network layer module is present or the "
                          "used network layer module does not subclass from "
                          "BaseNetworkLayer.");
            }
        }
        const AddressingInterface *const addrScheme = FindModule<AddressingInterface*>::findSubModule(pHost);
        if(addrScheme) {
            myAppAddr = addrScheme->myNetwAddr(netw);
        } else {
            myAppAddr = LAddress::L3Type( netw->getId() );
        }

        debugEV << "myAppAddr = " << myAppAddr << endl;
    }
}

MobileNodeAppLayerHoHuT::~MobileNodeAppLayerHoHuT() {}

void MobileNodeAppLayerHoHuT::finish()
{
    cancelAndDelete(selfTimer);
    if(calibrationMode)
    {
        xmlSaveFormatFileEnc("xml_data_files/radioMap.xml",convertRadioMapToXML(),"UTF-8",1);
        xmlSaveFormatFileEnc("xml_data_files/radioMapClusters.xml",convertRadioMapClustersToXML(),"UTF-8",1);
        xmlCleanupParser();
        xmlMemoryDump();
    }
}

//HANDLING MSGS
void MobileNodeAppLayerHoHuT::handleSelfMsg(cMessage * msg)
{
    switch(msg->getKind())
    {
        case SEND_COLLECTED_DATA_TIMER:
            sendCollectedDataToBaseStations();
            break;
        default:
            error ("Unknown msg kind");
            delete msg;
            break;
    }
}
void MobileNodeAppLayerHoHuT::handleLowerMsg(cMessage * msg)
{
    NetwToApplControlInfo* cInfo;
    HoHuTApplPkt* applPkt;

    switch (msg->getKind())
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
            //error("Unknown message of kind: "+msg->getKind());
            debugEV << "Unknown msg kind received! Deleting!" << endl;
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
        radioMapPosition_t* radioMapPosition=NULL;

        for (unsigned int i=0; i<staticNodeAddrCollected.size();i++)
        {
            if (staticNodesRSSITable.count(staticNodeAddrCollected[i])>=minimumStaticNodesForSample)
            {
                cStdDev stat("staticNodeStat");
                addressRSSIMultiMap_t::iterator it;
                std::pair<addressRSSIMultiMap_t::iterator,addressRSSIMultiMap_t::iterator> ppp;

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

        if (radioMapPosition!=NULL && !hasCollectedPosition(radioMapPosition->pos))
        {
            if (radioMapPosition->staticNodesPDFSet->size() < maxPositionPDFsSize)
            {
                error ("Please decrement the maxStaticNodesPDFSetXMLSize config.");
            }
            radioMap.insert(radioMapPosition);
            clusterizeRadioMapPosition(radioMapPosition);
        }

        staticNodesRSSITable.clear();
        previousPosition = currentPosition;
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
void MobileNodeAppLayerHoHuT::sendCollectedDataToBaseStations()
{
    debugEV << "preparing to send collected data!" << endl;

    addressVec_t::iterator i;
    addressRSSIMap_t::iterator j;
    addressRSSIMap_t* collectedData = new staticNodeSigsMap_t;

    for (unsigned int i=0; i<staticNodeAddrCollected.size();i++)
    {
        if (staticNodesRSSITable.count(staticNodeAddrCollected[i])>=minimumStaticNodesForSample)
        {
            cStdDev stat("staticNodeStat");
            addressRSSIMultiMap_t::iterator it;
            std::pair<addressRSSIMultiMap_t::iterator,addressRSSIMultiMap_t::iterator> ppp;

            ppp = staticNodesRSSITable.equal_range(staticNodeAddrCollected[i]);
            for (it=ppp.first; it != ppp.second; ++it)
            {
                stat.collect((*it).second);
            }

            collectedData->insert(std::pair<LAddress::L3Type,double>(staticNodeAddrCollected[i],convertTodBm(stat.getMean())));
            staticNodesRSSITable.erase(staticNodeAddrCollected[i]);

            debugEV << "RSSI was collected for node: " << staticNodeAddrCollected[i] << " and will be sent to all available base stations" << endl;
        }
    }

    if (collectedData->size()>0)
    {
        double maxRSSI;
        LAddress::L3Type maxRSSIDestAddr = -1;
        for (j=collectedData->begin();j!=collectedData->end();j++)
        {
            if (maxRSSIDestAddr==-1)
            {
                maxRSSI = j->second;
                maxRSSIDestAddr = j->first;
            }
            else if (maxRSSI != std::max(maxRSSI,j->second))
            {
                maxRSSI = j->second;
                maxRSSIDestAddr = j->first;
            }
        }

        HoHuTApplPkt* appPkt = new HoHuTApplPkt("collected-rssi",MOBILE_NODE_MSG);
        appPkt->setSrcAppAddress(myAppAddr);
        appPkt->setPayload("test 5");
        appPkt->setHoHuTMsgType(COLLECTED_RSSI);
        appPkt->setCollectedRSSIs(*collectedData);
        appPkt->setRealPosition(currentPosition);
        NetwControlInfo::setControlInfo(appPkt,maxRSSIDestAddr);
        sendDown(appPkt);

        debugEV << "collected-rssi sent to maxRSSI node:" << maxRSSIDestAddr << endl;
    }

    if (!selfTimer->isScheduled())
    {
        scheduleAt(simTime()+collectedDataSendingTimePeriod,selfTimer);
    }
}
void MobileNodeAppLayerHoHuT::sendMobileNodeMsg(char* msgPayload, LAddress::L3Type netwDestAddr)
{
    debugEV << "Sending mobile node msg to netwDestAddr:" << netwDestAddr << endl;

    HoHuTApplPkt* appPkt = new HoHuTApplPkt("mobile-node-app-msg",MOBILE_NODE_MSG);
    appPkt->setPayload(msgPayload);
    NetwControlInfo::setControlInfo(appPkt, netwDestAddr);
    sendDown(appPkt);
}


//// RADIO MAP & CLUSTERS
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
bool MobileNodeAppLayerHoHuT::hasCollectedPosition(Coord pos)
{
    radioMapSet_t::iterator radioMapIt;
    for (radioMapIt=radioMap.begin(); radioMapIt!=radioMap.end();radioMapIt++)
    {
        radioMapPosition_t* radioMapPosition = *radioMapIt;
        if (radioMapPosition->pos == pos)
        {
            return true;
        }
    }
    return false;
}
void MobileNodeAppLayerHoHuT::clusterizeRadioMapPosition(radioMapPosition_t* radioMapPosition)
{
    staticNodesPDFSet_t::iterator it;
    addressVec_t* positionKey = new addressVec_t;
    for (it=radioMapPosition->staticNodesPDFSet->begin(); it!=radioMapPosition->staticNodesPDFSet->end();it++)
    {
        positionKey->push_back((*it)->addr);
        if (positionKey->size()==clusterKeySize)
        {
            break;
        }
    }
    if (positionKey->size()!=clusterKeySize)
    {
        error("Error creating cluster key for position. Please decrement the clusterKeySize in the config file.");
    }
    coordVec_t* clusterCoordsSet = getCoordSetByClusterKey(positionKey);
    if (clusterCoordsSet==NULL)
    {
        clusterCoordsSet = new coordVec_t;
        clusterCoordsSet->push_back(new Coord(radioMapPosition->pos.x,radioMapPosition->pos.y,radioMapPosition->pos.z));
        radioMapClusters.insert(std::pair<addressVec_t*,coordVec_t*>(positionKey,clusterCoordsSet));
    }
    else
    {
        clusterCoordsSet->push_back(new Coord(radioMapPosition->pos.x,radioMapPosition->pos.y,radioMapPosition->pos.z));
    }
}
MobileNodeAppLayerHoHuT::coordVec_t* MobileNodeAppLayerHoHuT::getCoordSetByClusterKey(addressVec_t* clusterKey)
{
    radioMapCluster_t::iterator it;
    addressVec_t* cKey;
    addressVec_t::iterator it2;
    addressVec_t::iterator it3;
    unsigned int numFound;

    for (it=radioMapClusters.begin();it!=radioMapClusters.end();it++)
    {
        cKey = it->first;
        numFound=0;
        for (it2=clusterKey->begin(); it2!=clusterKey->end();it2++)
        {
            for (it3=cKey->begin(); it3!=cKey->end(); it3++)
            {
                if (*it2==*it3)
                {
                    numFound++;
                }
            }
        }
        if (numFound==clusterKey->size())
        {
            return it->second;
        }
    }
    return NULL;
}
xmlDocPtr MobileNodeAppLayerHoHuT::convertRadioMapToXML()
{
    radioMapSet_t::iterator it;
    staticNodesPDFSet_t::iterator it2;

    xmlDocPtr xmlDoc = xmlNewDoc((const xmlChar*) "1.0");
    xmlNodePtr xmlDocRoot = xmlNewNode(NULL,(const xmlChar*) "radioMap");
    xmlNewProp(xmlDocRoot,(const xmlChar*) "maxPositionPDFsSize",convertNumberToXMLString(maxPositionPDFsSize));
    xmlDocSetRootElement(xmlDoc, xmlDocRoot);

    for (it=radioMap.begin(); it!=radioMap.end();it++)
    {
        unsigned int addedPDFs = 0;
        radioMapPosition_t* position = *it;
        xmlNodePtr positionNode = xmlNewNode(NULL,(const xmlChar*) "position");
        xmlNewProp(positionNode, (const xmlChar*) "x", convertNumberToXMLString(position->pos.x));
        xmlNewProp(positionNode, (const xmlChar*) "y", convertNumberToXMLString(position->pos.y));

        staticNodesPDFSet_t* staticNodes = position->staticNodesPDFSet;
        for (it2=staticNodes->begin(); it2!=staticNodes->end() && addedPDFs<maxPositionPDFsSize;it2++)
        {
            staticNodePDF_t* sig = *it2;
            xmlNodePtr staticNodePDFXMLNode = xmlNewNode(NULL,(const xmlChar*) "staticNodePDF");
            xmlNewProp(staticNodePDFXMLNode,(const xmlChar*) "address", convertNumberToXMLString(sig->addr));
            xmlNewProp(staticNodePDFXMLNode,(const xmlChar*) "mean", convertNumberToXMLString(sig->mean));
            xmlNewProp(staticNodePDFXMLNode,(const xmlChar*) "stdDev", convertNumberToXMLString(sig->stdDev));
            xmlAddChild(positionNode,staticNodePDFXMLNode);
            addedPDFs++;
        }
        xmlAddChild(xmlDocRoot,positionNode);
    }

    return xmlDoc;
}
xmlDocPtr MobileNodeAppLayerHoHuT::convertRadioMapClustersToXML()
{
    radioMapCluster_t::iterator it;
    coordVec_t::iterator it2;
    addressVec_t::iterator it4;

    addressVec_t* clusterKey;
    coordVec_t* positions;

    xmlNodePtr clusterNode;
    xmlNodePtr clusterKeyNode;
    xmlNodePtr clusterKeyStaticNode;
    xmlNodePtr positionNode;

    LAddress::L3Type staticNodeAddress;

    xmlDocPtr xmlDoc = xmlNewDoc((const xmlChar*) "1.0");
    xmlNodePtr xmlDocRoot = xmlNewNode(NULL,(const xmlChar*) "radioMapClusters");
    xmlNewProp(xmlDocRoot,(const xmlChar*) "clusterKeySize",convertNumberToXMLString(clusterKeySize));
    xmlDocSetRootElement(xmlDoc, xmlDocRoot);

    for(it=radioMapClusters.begin();it!=radioMapClusters.end();it++)
    {
        clusterKey = it->first;
        clusterNode = xmlNewNode(NULL, (const xmlChar*)"cluster");
        clusterKeyNode = xmlNewNode(NULL, (const xmlChar*) "clusterKey");
        for (it4=clusterKey->begin(); it4!=clusterKey->end();it4++)
        {
            staticNodeAddress = *it4;
            clusterKeyStaticNode = xmlNewNode(NULL, (const xmlChar*) "staticNode");
            xmlNewProp(clusterKeyStaticNode, (const xmlChar*) "address", convertNumberToXMLString(staticNodeAddress));
            xmlAddChild(clusterKeyNode,clusterKeyStaticNode);
        }
        xmlAddChild(clusterNode,clusterKeyNode);

        positions = it->second;
        for (it2=positions->begin(); it2!=positions->end(); it2++)
        {
            Coord position = (*it2);
            positionNode = xmlNewNode(NULL, (const xmlChar*) "position");
            xmlNewProp(positionNode, (const xmlChar*) "x", convertNumberToXMLString(position.x));
            xmlNewProp(positionNode, (const xmlChar*) "y", convertNumberToXMLString(position.y));
            xmlAddChild(clusterNode,positionNode);
        }

        xmlAddChild(xmlDocRoot,clusterNode);
    }
    return xmlDoc;
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

const xmlChar* MobileNodeAppLayerHoHuT::convertNumberToXMLString(double number)
{
    std::ostringstream* s = new std::ostringstream;
    *s << number;
    std::string str = s->str();
    const char* string = str.c_str();
    return (const xmlChar*)string;
}
