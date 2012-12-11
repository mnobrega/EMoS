package org.mixim.modules.node.EMoS;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseWorldUtility;
import org.mixim.base.modules.IBaseNetwLayer;
import org.mixim.modules.application.EMoS.StaticNodeAppLayerEMoS;
import org.mixim.modules.netw.ArpHost;
import org.mixim.modules.nic.Nic802154_XBee;
import org.mixim.modules.power.battery.BatteryStats;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.transport.Aggregation;


// This model defines the layer structure of the static node in EMoS
// @author Márcio Nóbrega

module Host802154_2400MHz_EMoS_static
{
    parameters:
        double numHosts; // total number of hosts in the network
        string netwType = default("DummyRoute"); //type of the network layer

        @display("bgb=356,318,white;bgp=10,10;b=40,40,rect;i=device/usb");
    gates:
        input radioIn;

    submodules:
        utility: BaseWorldUtility {
            parameters:
                @display("p=292,60;i=block/blackboard");
        }
        batteryStats: BatteryStats {
            @display("p=292,257;i=block/circle");
            detail = default(false);
            timeSeries = default(false);
        }
        battery: SimpleBattery {
            @display("p=292,186;i=block/control");
            nominal = default(1000 mAh);
            capacity = default(1000 mAh);
            voltage = default(3 V);
            resolution = default(60 s);
            publishDelta = default(1);
            publishTime = default(60 s);
            numDevices = default(1);
        }
        mobility: BaseMobility {
            parameters:
                @display("p=292,120;i=block/cogwheel");
        }
        nic: Nic802154_XBee {
            parameters:
                @display("b=32,30;p=86,265;i=block/ifcard");
        }
        net: <netwType> like IBaseNetwLayer {
            parameters:
                @display("p=86,201");
        }
        transport: Aggregation {
            @display("p=86,132");
        }
        arp: ArpHost {
            parameters:
                @display("p=202,186");
        }
        app: StaticNodeAppLayerEMoS {
            parameters:
                @display("p=86,38;i=block/app");

        }

    connections:
        net.lowerLayerOut --> nic.upperLayerIn;
        net.lowerLayerIn <-- nic.upperLayerOut;
        net.lowerControlOut --> nic.upperControlIn;
        net.lowerControlIn <-- nic.upperControlOut;

        net.upperLayerOut --> transport.lowerLayerIn;
        net.upperLayerIn <-- transport.lowerLayerOut;
        net.upperControlOut --> transport.lowerControlIn;
        net.upperControlIn <-- transport.lowerControlOut;

        transport.upperLayerOut --> app.lowerLayerIn;
        transport.upperLayerIn <-- app.lowerLayerOut;
        transport.upperControlOut --> app.lowerControlIn;
        transport.upperControlIn <-- app.lowerControlOut;


        radioIn --> nic.radioIn;
}

