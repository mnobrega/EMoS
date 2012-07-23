package org.mixim.modules.application.HoHuT;

simple MobileNodeAppLayer
{
    parameters:
        bool debug = default(false); 		    	// debug mode
        int nodeAddr;								// node address
        bool dataCollectionMode = default(false);	//data collection mode active
        
    gates:
        input lowerGateIn; // to receive data from communications stack
        output lowerGateOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack

}
