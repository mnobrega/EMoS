package org.mixim.modules.netw.HoHuT;

import org.mixim.base.modules.BaseNetwLayer;

simple AODVRoute extends BaseNetwLayer
{
    parameters:
        bool trace = default(false);
        bool debug = default(false);
        stats = default(true);
        headerLength = default(0);
        int RREQVectorMaxSize = default(25);
        int RREQVectorElementMaxLifetime = default(30); //0.5 min
        int RREQVectorMaintenancePeriod = default(60); //1 min
        int routeMapElementMaxLifetime = default(1800); //30 min
        bool localRepair = default(false);
        @display("i=block/fork");
        @class(AODVRoute);
}