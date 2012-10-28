package org.mixim.modules.application.EMoS;

simple StaticNodeAppLayerEMoS
{
    parameters:
        bool debug = default(false); 			
        int headerLength = default(24);
        
        int nodeSigPeriod = default(1);
        int nodeSigStartingTime = default(5);
        bool beaconMode = default(false); 		//this mode sets this node as a beacon sending node sigs
        int baseStationNetwAddr = default(0); 	
        
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
