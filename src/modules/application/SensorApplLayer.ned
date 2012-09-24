package org.mixim.modules.application;

import org.mixim.base.modules.IBaseApplLayer;


// @brief Application layer to test lower layer implementations. 
// SensorApplLayer waits a time initializationTime, then starts sending
// nbPackets (of size headerLength) to host destAddr. The time intervals
// between two packets can follow three distributions: periodic (fixed),
// uniform or exponential.
//
// @ingroup application
// @author Jerome Rousselot, Amre El-Hoiydi
simple SensorApplLayer like IBaseApplLayer
{
    parameters:
        bool debug = default(false); // debug switch
        bool stats = default(true);   // stats generation switch
        bool trace = default(false);  // activates detailed logging (per source latencies and vector logging). stats must be true.
        bool broadcastPackets = default(false); // send packets in broadcast mode
        double nbPackets = default(0);
        int destAddr = default(0);
        string trafficType = default("periodic");  	// Can be one of: periodic, uniform or exponential
        double trafficParam @unit(s) = default(1 s);			// the mean time between two packets
        double initializationTime @unit(s) = default(1 min); // minimum time before generation of the first packet
        int headerLength @unit(byte) = default(2 byte);
        bool notAffectedByHostState = default(true);
    gates:
        input lowerLayerIn; // from network layer
        input lowerControlIn;
        output lowerLayerOut; // to network layer
        output lowerControlOut;
}
