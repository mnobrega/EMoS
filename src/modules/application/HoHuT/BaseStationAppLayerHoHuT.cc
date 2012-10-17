#include "BaseStationAppLayerHoHuT.h"

Define_Module(BaseStationAppLayerHoHuT);

void BaseStationAppLayerHoHuT::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if (stage == 0)
    {
    	debugEV<< "in initialize() stage 0...";
        lowerLayerOut = findGate("lowerLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerControlOut = findGate("lowerControlOut");
        lowerControlIn = findGate("lowerControlIn");
        debug = par("debug").boolValue();
        useClustering = par("useClustering").boolValue();
        radioMapXML = par("radioMapXML");
        radioMapClustersXML = par("radioMapClustersXML");
        normalStandardDistribXML = par("normalStandardDistribXML");
    }
    else if (stage == 1)
    {
    	debugEV << "in initialize() stage 1...";
    	loadRadioMapFromXML(radioMapXML);
    	loadRadioMapClustersFromXML(radioMapClustersXML);
    	loadNormalStandard(normalStandardDistribXML);
    }
}

BaseStationAppLayerHoHuT::~BaseStationAppLayerHoHuT() {}

void BaseStationAppLayerHoHuT::finish()
{

}

void BaseStationAppLayerHoHuT::handleSelfMsg(cMessage * msg)
{
    switch (msg->getKind())
    {
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}

void BaseStationAppLayerHoHuT::handleLowerMsg(cMessage * msg)
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
            handleMobileNodeMsg(msg);
            break;
        case STATIC_NODE_SIG: //static nodes ignore this msg types. they are only for the mobile nodes
            delete msg;
            break;
        default:
            error("Unknown message of kind: "+msg->getKind());
            delete msg;
            break;
    }
}
void BaseStationAppLayerHoHuT::handleLowerControl(cMessage* msg)
{
    delete msg; //dont do nothing for now
}

//Note: mobile node msgs are always sent through static nodes
void BaseStationAppLayerHoHuT::handleMobileNodeMsg(cMessage* msg)
{
    NetwToApplControlInfo* cInfo;
    HoHuTApplPkt* applPkt;
    cInfo = static_cast<NetwToApplControlInfo*>(msg->removeControlInfo());
    applPkt = static_cast<HoHuTApplPkt*>(msg);
    debugEV << "received rssi: " << cInfo->getRSSI() << endl;
    debugEV << "Received a node msg from mobile node with appAddr: " << applPkt->getSrcAppAddress() << endl;
    debugEV << "msg data:" << applPkt->getPayload() << endl;

    staticNodeSamplesSet_t* collectedRSSIs = getOrderedCollectedRSSIs(static_cast<addressRSSIMap_t*>(&(applPkt->getCollectedRSSIs())));
    Coord mobileNodePosition = getNodeLocation(collectedRSSIs);

    delete msg;
}

//LOCALIZATION
Coord BaseStationAppLayerHoHuT::getNodeLocation(staticNodeSamplesSet_t* staticNodeSamples)
{
    radioMapSet_t::iterator position;
    staticNodesPDFSet_t::iterator staticNodePDF;
    radioMapSet_t* candidatePositions = getCandidatePositions(staticNodeSamples);

    for (position=candidatePositions->begin(); position!=candidatePositions->end();position++)
    {
        radioMapPosition_t* rMapPosition = *position;
        Coord curCoord = rMapPosition->pos;
        staticNodesPDFSet_t* staticNodesPDFs = rMapPosition->staticNodesPDFSet;
        for (staticNodePDF=staticNodesPDFs->begin(); staticNodePDF!=staticNodesPDFs->end();staticNodePDF++)
        {
            //TODO - for each staticNodePDF calculate the P

            //if P > max then x=l and max=p
        }
    }
}
BaseStationAppLayerHoHuT::radioMapSet_t* BaseStationAppLayerHoHuT::getCandidatePositions(staticNodeSamplesSet_t* staticNodeSamples)
{
    if (useClustering)
    {
        staticNodeSamplesSet_t::iterator nodeSample;
        unsigned int nodeNumber = 0;
        addressVec_t* sampleKey = new addressVec_t;
        for (nodeSample=staticNodeSamples->begin();nodeSample!=staticNodeSamples->end() && nodeNumber < clusterKeySize; nodeSample++)
        {
            sampleKey->push_back((*nodeSample)->addr);
            nodeNumber++;
        }

        coordVec_t* clusterPositions = getCoordSetByClusterKey(sampleKey);
        if (clusterPositions!=NULL)
        {
            radioMapSet_t* candidatePositions = new radioMapSet_t;
            radioMapSet_t::iterator rMapPos;
            coordVec_t::iterator coord;
            for (coord=clusterPositions->begin();coord!=clusterPositions->end();coord++)
            {
                Coord pos = *coord;
                for (rMapPos=radioMap.begin(); rMapPos!=radioMap.end();rMapPos++)
                {
                    radioMapPosition_t* radioMapPosition = *rMapPos;
                    if (radioMapPosition->pos==pos)
                    {
                        candidatePositions->insert(radioMapPosition);
                    }
                }
            }
            return candidatePositions;
        }
        else
        {
            error("Radio map has no information for the specified key. This shouldnt happen.");
        }
    }
    return &radioMap;
}
BaseStationAppLayerHoHuT::coordVec_t* BaseStationAppLayerHoHuT::getCoordSetByClusterKey(addressVec_t* clusterKey)
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


//XML FILES
void BaseStationAppLayerHoHuT::loadNormalStandard(cXMLElement* xml)
{
    cXMLElementList::const_iterator row;
    cXMLElementList::const_iterator cell;

    std::string rootTag = xml->getTagName();
    ASSERT(rootTag=="table");

    cXMLElementList rows = xml->getChildren();
    for (row=rows.begin();row!=rows.end();++row)
    {
        cXMLElement* cell;
        double zValue;
        double normalDist;

        cXMLElementList cells = (*row)->getChildren();
        for (unsigned int i=0; i<cells.size();i++)
        {
            if (i==0)
            {
                cell = cells[i];
                zValue = convertStringToNumber(cell->getAttribute("value"));
            }
            else if (i==1)
            {
                cell = cells[i];
                normalDist = convertStringToNumber(cell->getAttribute("value"));
            }
            else
            {
                error("Too many cells in row for the standard normal XML");
            }
        }
        normalStandardTable.insert(std::pair<double,double>(zValue,normalDist));
    }
}
void BaseStationAppLayerHoHuT::loadRadioMapFromXML(cXMLElement* xml)
{
    cXMLElementList::const_iterator i;
    cXMLElementList::const_iterator j;

    std::string rootTag = xml->getTagName();
    ASSERT(rootTag=="radioMap");

    cXMLElementList positions = xml->getChildren();
    for (i=positions.begin();i!=positions.end();++i)
    {
        //<position x="9.5" y="17">
        cXMLElement* position = *i;
        radioMapPosition* radioMapPosition = (struct radioMapPosition*) malloc(sizeof(struct radioMapPosition));
        ASSERT(position->getAttribute("x"));
        radioMapPosition->pos.x = convertStringToNumber(position->getAttribute("x"));
        ASSERT(position->getAttribute("y"));
        radioMapPosition->pos.y = convertStringToNumber(position->getAttribute("y"));
        radioMapPosition->pos.z = 0;
        radioMapPosition->staticNodesPDFSet = new staticNodesPDFSet_t;


        cXMLElementList staticNodePDFs = position->getChildren();
        for (j=staticNodePDFs.begin();j!=staticNodePDFs.end();++j)
        {
            //<staticNodePDF address="1001" mean="-22.37" stdDev="-21.4491"/>
            cXMLElement* staticNodePDF = *j;
            staticNodePDF_t* radioMapStaticNodePDF = (struct staticNodePDF*) malloc(sizeof(struct staticNodePDF));
            ASSERT(staticNodePDF->getAttribute("address"));
            radioMapStaticNodePDF->addr = convertStringToNumber(staticNodePDF->getAttribute("address"));
            ASSERT(staticNodePDF->getAttribute("mean"));
            radioMapStaticNodePDF->mean = convertStringToNumber(staticNodePDF->getAttribute("mean"));
            ASSERT(staticNodePDF->getAttribute("stdDev"));
            radioMapStaticNodePDF->stdDev = convertStringToNumber(staticNodePDF->getAttribute("stdDev"));
            radioMapPosition->staticNodesPDFSet->insert(radioMapStaticNodePDF);
        }

        radioMap.insert(radioMapPosition);
    }
}

void BaseStationAppLayerHoHuT::loadRadioMapClustersFromXML(cXMLElement* xml)
{
    cXMLElementList::const_iterator i;
    cXMLElementList::const_iterator j;
    cXMLElementList::const_iterator k;

    std::string rootTag = xml->getTagName();
    ASSERT(rootTag=="radioMapClusters");

    clusterKeySize = convertStringToNumber(xml->getAttribute("clusterKeySize"));

    cXMLElementList clusters = xml->getChildren();
    for (i=clusters.begin();i!=clusters.end();++i)
    {
        cXMLElement* cluster = *i;
        cXMLElementList clusterElements = cluster->getChildren();
        coordVec_t* radioMapPositions = new coordVec_t;
        addressVec_t* radioMapClusterKey;
        for (j=clusterElements.begin(); j!=clusterElements.end();++j)
        {
            cXMLElement* clusterElement = *j;

            if (strcmp(clusterElement->getTagName(),"clusterKey")==0)
            {
                cXMLElementList clusterKey = clusterElement->getChildren();
                radioMapClusterKey = new addressVec_t;
                for (k=clusterKey.begin(); k!=clusterKey.end(); ++k)
                {
                    //<staticNode address="1000"/>
                    cXMLElement* staticNode = *k;
                    ASSERT(staticNode->getAttribute("address"));
                    radioMapClusterKey->push_back(convertStringToNumber(staticNode->getAttribute("address")));
                }
            }

            //<position x="9.5" y="15"/>
            if (strcmp(clusterElement->getTagName(),"position")==0)
            {
                cXMLElement* position = clusterElement;
                ASSERT(position->getAttribute("x"));
                ASSERT(position->getAttribute("y"));
                Coord radioMapPosition = new Coord(convertStringToNumber(position->getAttribute("x")),convertStringToNumber(position->getAttribute("y")),0);
                radioMapPositions->push_back(radioMapPosition);
            }
        }

        radioMapClusters.insert(std::pair<addressVec_t*,coordVec_t*>(radioMapClusterKey,radioMapPositions));
    }
}

//AUX
BaseStationAppLayerHoHuT::staticNodeSamplesSet_t* BaseStationAppLayerHoHuT::getOrderedCollectedRSSIs(addressRSSIMap_t* appPktCollectedRSSIs)
{
    addressRSSIMap_t::iterator i;
    staticNodeSamplesSet_t* orderedCollectedRSSIs = new staticNodeSamplesSet_t;

    for (i=appPktCollectedRSSIs->begin();i!=appPktCollectedRSSIs->end();i++)
    {
        staticNodeRSSISample_t* staticNodePDF = (struct staticNodeRSSISample*) malloc(sizeof(struct staticNodeRSSISample));
        staticNodePDF->addr = i->first;
        staticNodePDF->mean = i->second;
        orderedCollectedRSSIs->insert(staticNodePDF);
    }
    return orderedCollectedRSSIs;
}
double BaseStationAppLayerHoHuT::convertStringToNumber(const std::string& str)
{
    std::istringstream i(str);
    double x;
    if (!(i >> x))
    {
        return 0;
    }
    return x;
}
