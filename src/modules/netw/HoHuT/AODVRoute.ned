package org.mixim.modules.netw.HoHuT;

import org.mixim.base.modules.BaseNetwLayer;

simple AODVRoute extends BaseNetwLayer
{
    parameters:
        bool trace = default(false);
        bool debug = default(false);
        stats = default(true);
        headerLength = default(0);
        int maxRREQVectorSize = default(25);
        int maxRREQVectorElementLifetime = default(30);
        int RREQSetMaintenancePeriod = default(60);
        @display("i=block/fork");
        @class(AODVRoute);
}