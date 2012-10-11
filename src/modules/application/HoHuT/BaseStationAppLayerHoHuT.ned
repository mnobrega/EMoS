package org.mixim.modules.application.HoHuT;

simple BaseStationAppLayerHoHuT
{
    parameters:
        bool debug = default(false); 			
        int headerLength = default(24);
        bool useClustering = default(true);
        xml radioMapXML = default(xml("<radioMap/>"));
        xml radioMapClustersXML = default(xml("<radioMapClusters/>"));
        
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
