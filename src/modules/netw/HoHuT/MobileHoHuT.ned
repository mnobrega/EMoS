package org.mixim.modules.netw.HoHuT;

import org.mixim.base.modules.BaseNetwLayer;

simple MobileHoHuT extends BaseNetwLayer
{
    parameters:
        bool trace = default(false);
        stats = default(true);
        headerLength = default(0);
        @class(MobileHoHuT);
}