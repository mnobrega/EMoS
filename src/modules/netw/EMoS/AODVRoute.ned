package org.mixim.modules.netw.EMoS;
//***************************************************************************
// file:        AODVRoute.ned
//
// author:      Márcio Nóbrega
// 
// copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
//
// description:  AODV (Ad-hoc On-demand Vector Routing)
// 
// **************************************************************************/

import org.mixim.base.modules.BaseNetwLayer;

simple AODVRoute extends BaseNetwLayer
{
    parameters:
        stats = default(true);
        headerLength = default(0);
        
        bool trace = default(false);
        bool debug = default(false);
        bool localRepair = default(false);
        int RREQVectorMaxSize = default(25);
        int RREQVectorElementMaxLifetime = default(30); //0.5 min
        int RREQVectorMaintenancePeriod = default(60); //1 min
        int routeMapElementMaxLifetime = default(1800); //30 min
        int packetMapMaxPktQueueElementLifetime = default(20);
        int packetMapMaintenancePeriod = default(10);
        
        @display("i=block/fork");
        @class(AODVRoute);
}