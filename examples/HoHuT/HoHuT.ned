package org.mixim.examples.HoHuT;

import org.mixim.modules.node.HoHuT.Host802154_2400MHz_HoHuT_base;
import org.mixim.modules.node.HoHuT.Host802154_2400MHz_HoHuT_static;
import org.mixim.modules.node.HoHuT.Host802154_2400MHz_HoHuT_mobile;

import org.mixim.modules.obstacle.ObstacleControl;
import org.mixim.modules.world.annotations.AnnotationManager;
import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


network HoHuT
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

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white;bgp=10,50;bgs=50,m");

    submodules:
        connectionManager: ConnectionManager {
            @display("p=30,5;b=0,0,rect,yellow;i=abstract/multicast;is=s");
        }
        annotations: AnnotationManager {

            @display("p=30,10");
        }
        obstacles: ObstacleControl {
            @display("p=30,15");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
            @display("p=30,20;i=misc/globe");
        }
        staticNode[numHostsStatic]: Host802154_2400MHz_HoHuT_static {
            parameters:
                numHosts = numHostsStatic;
            @display("i=micaz_s;p=12,12.5;b=1,1;is=n");
        }
        mobileNode[numHostsMobile]: Host802154_2400MHz_HoHuT_mobile {
            parameters:
                numHosts = numHostsMobile;
            @display("i=device/palm2_s;p=10,12.5;b=0,0;is=n");
        }
        baseStation[numHostsBase]: Host802154_2400MHz_HoHuT_base {
            parameters:
                numHosts = numHostsBase;
        	@display("i=device/pc2_s;p=10,12.5;b=0,0;is=n");
        }

    connections allowunconnected:
}
