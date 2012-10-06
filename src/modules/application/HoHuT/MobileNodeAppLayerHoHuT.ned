package org.mixim.modules.application.HoHuT;

simple MobileNodeAppLayerHoHuT
{
    parameters:
        bool debug = default(false);
        bool stats = default(true);										// gather statistical data
        //general AODV stuff		
        int headerLength = default(24);
        int nodeSigPeriod = default(60);
        int nodeSigStartingTime = default(5);
        int packetMapMaxPktQueueElementLifetime = default(20);
        int packetMapMaintenancePeriod = default(10);
        //mobile node stuff
        int minimumStaticNodesForSample = default(10);					// static node sigs received limit per static node
        int clusterKeySize = default(2);								// size of the cluster KEY
        bool calibrationMode = default(false);							// calibration of the static node position table 	
        
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
