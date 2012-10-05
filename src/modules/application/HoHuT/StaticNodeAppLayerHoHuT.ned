package org.mixim.modules.application.HoHuT;

simple StaticNodeAppLayerHoHuT
{
    parameters:
        bool debug = default(false); 			
        int headerLength = default(24);
        int nodeSigPeriod = default(60);
        int nodeSigStartingTime = default(5);
        int packetMapMaxPktQueueElementLifetime = default(20);
        int packetMapMaintenancePeriod = default(10);
        
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
