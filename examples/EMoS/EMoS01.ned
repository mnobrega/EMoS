package org.mixim.examples.EMoS;

import org.mixim.modules.node.EMoS.Host802154_2400MHz_EMoS_base;
import org.mixim.modules.node.EMoS.Host802154_2400MHz_EMoS_static;
import org.mixim.modules.node.EMoS.Host802154_2400MHz_EMoS_mobile;

import org.mixim.modules.obstacle.ObstacleControl;
import org.mixim.modules.world.annotations.AnnotationManager;
import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


network EMoS01
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHostsBase;	//total number of base stations in the network
        double numHostsStatic; // total number of static hosts in the network
        double numHostsMobile; // total number of mobile hosts in the network

        @signal[rssiVal](type="long");
        @statistic[arrivedrssi](title="Arrived RSSI"; source="rssiVal"; record=vector; interpolationmode=none);

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white;bgp=10,50;bgs=101,m;bgi=home01");

    submodules:
        connectionManager: ConnectionManager {
            @display("p=13.607843,4.7156863;b=0,0,rect,yellow;i=abstract/multicast;is=s");
        }
        annotations: AnnotationManager {

            @display("p=13.568627,3.3039215");
        }
        obstacles: ObstacleControl {
            @display("p=13.568627,2.0196078");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=13.558824,0.65686274;i=misc/globe");
        }
        staticNode[numHostsStatic]: Host802154_2400MHz_EMoS_static {
            parameters:
                numHosts = numHostsStatic;
                @display("i=micaz_s;p=4.038835,5.3592234;b=0,0;is=n");
        }
        mobileNode[numHostsMobile]: Host802154_2400MHz_EMoS_mobile {
            parameters:
                numHosts = numHostsMobile;
                @display("i=wasp_s;p=1.9805825,5.3592234;b=0,0;is=n");
        }
        baseStation[numHostsBase]: Host802154_2400MHz_EMoS_base {
            parameters:
                numHosts = numHostsBase;
                @display("i=device/pc2_vl;p=0.4563107,0.5048544;b=0,0;is=l");
        }

    connections allowunconnected:
}
