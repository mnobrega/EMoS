package org.mixim.modules.application.HoHuT;

simple BaseStationAppLayerHoHuT
{
    parameters:
        bool debug = default(false); 
        bool recordStats = default(false);		
        int headerLength = default(24);
        bool useClustering = default(false);
        bool useCenterOfMass = default(false);
        bool usePhySpaceTimeAvg = default(false);
        int maxCenterOfMassPositions = default(1);
        int phySpaceTimeAvgWindowSize = default(1);
        xml radioMapXML = default(xml("<radioMap/>"));
        xml radioMapClustersXML = default(xml("<radioMapClusters/>"));
        xml normalStandardDistribXML = default(xml("<table/>"));
        
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
