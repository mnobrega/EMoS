package org.mixim.modules.netw.HoHuT;

import org.mixim.base.modules.BaseNetwLayer;

simple AODVRoute extends BaseNetwLayer
{
    parameters:
        bool trace = default(false);
        stats = default(true);
        headerLength = default(0);
        @display("i=block/fork");
        @class(AODVRoute);
}