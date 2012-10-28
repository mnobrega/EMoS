package org.mixim.modules.application.EMoS;

simple MobileNodeAppLayerEMoS
{
    parameters:
        bool debug = default(false);
        bool stats = default(true);										// gather statistical data
        int headerLength = default(24);
        int minimumStaticNodesForSample = default(10);					// static node sigs received limit per static node
        int clusterKeySize = default(2);								// size of the cluster KEY
        bool calibrationMode = default(false);							// calibration of the static node position table
        int collectedDataSendingTimePeriod = default(60);				// period to send a new collected sample to the base station
        double firstPosX = default(0);
        double firstPosY = default(0);
        int maxPositionPDFsSize = default(4);					//max PDFs for each radio map position
                
    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack
}
