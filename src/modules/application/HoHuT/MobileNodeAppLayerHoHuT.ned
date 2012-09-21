package org.mixim.modules.application.HoHuT;

simple MobileNodeAppLayerHoHuT
{
    parameters:
        bool debug = default(false); 		    						// debug mode
        bool stats = default(true);										// gather statistical data
        bool calibrationMode = default(false);							// calibration of the static node position table
        int minimumStaticNodesForSample = default(10);					// static node sigs received limit per static node
        int clusterKeySize = default(2);								// size of the cluster KEY
        
    gates:
        input lowerGateIn; // to receive data from communications stack
        output lowerGateOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack

}
