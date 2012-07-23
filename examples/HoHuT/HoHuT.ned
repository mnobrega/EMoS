package org.mixim.examples.HoHuT;
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import org.mixim.modules.node.HoHuT.Host802154_2400MHz_static;
import org.mixim.modules.node.HoHuT.Host802154_2400MHz_mobile;
import org.mixim.modules.obstacle.ObstacleControl;
import org.mixim.modules.world.annotations.AnnotationManager;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


//
// This example shows how to simulate wireless sensor networks.
// Three different routing protocols are considered, for different traffic 
// types:
// convergecast, with Wiseroute;
// network-level broadcast, with flooding;
// and probabilistic broadcast.
//
network HoHuT
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHostsStatic; // total number of static hosts in the network
        double numHostsMobile; // total number of mobile hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white;bgp=10,50;bgs=3,m;");

    submodules:
        connectionManager: ConnectionManager {
            parameters:
                @display("p=132.66667,50.333332;b=0,0,rect,yellow;i=abstract/multicast;is=s");
        }
        obstacles: ObstacleControl {
            @display("p=171.33333,25.333334");
        }
        annotations: AnnotationManager {

            @display("p=171.33333,50.333332");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=132.66667,25.333334;i=misc/globe");
        }
        staticNode[numHostsStatic]: Host802154_2400MHz_static {
            parameters:
                numHosts = numHostsStatic;
                @display("i=micaz_s;p=57.6,50.6;b=12,12");

        }
        mobileNode[numHostsMobile]: Host802154_2400MHz_mobile {
            parameters:
                numHosts = numHostsMobile;
                @display("i=device/palm2_s;p=57.4,32.6;b=6,6;is=n");
        }

    connections allowunconnected:
}
