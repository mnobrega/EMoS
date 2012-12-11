//**************************************************************************
// * file:        AODVData.msg
// *
// * author:      Márcio Nóbrega
// *
// * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
// *
// *           	  This message is used by the network layer to send/receive 
// * 			  data to/from the MAC layer.
// **************************************************************************
// * part of:     EMoS - Elder Monitoring System
// **************************************************************************/

cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVData extends NetwPkt
{
	LAddress::L3Type		finalDestAddr;		//final destination node
	LAddress::L3Type		initialSrcAddr;		//initial source node
}