package org.mixim.modules.application.HoHuT;

simple StaticNodeAppLayerHoHuT
{
    parameters:
        bool debug = default(false); 		// debug switch
        int  headerLength = default(24);	// length of the network packet header (in bits)
        int maxPktQueueElementLifetime = default(20);
        int pktQueueMaintenancePeriod = default(10);
        int maxPacketDeliveryTries = default(5);
        
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
