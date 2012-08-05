package org.mixim.modules.application.HoHuT;

simple StaticNodeAppLayerHoHuT
{
    parameters:
        bool debug = default(false); 		// debug switch
        
    gates:
        input lowerGateIn; // to receive data from communications stack
        output lowerGateOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
