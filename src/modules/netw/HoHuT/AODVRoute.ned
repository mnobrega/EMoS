package org.mixim.modules.netw.HoHuT;

import org.mixim.base.modules.BaseNetwLayer;

simple AODVRoute extends BaseNetwLayer
{
    parameters:
        bool trace = default(false);
        bool debug = default(false);
        stats = default(true);
        headerLength = default(0);
        int pktQueueElementLifetime = default(20);
        int pktQueueCheckingPeriod = default(10);
        @display("i=block/fork");
        @class(AODVRoute);
}