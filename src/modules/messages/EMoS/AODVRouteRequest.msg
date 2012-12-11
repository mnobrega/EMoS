//**************************************************************************
// * file:        AODVRouteRequest.msg
// *
// * author:      Márcio Nóbrega
// *
// * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
// *
// *           	  Message sent in broadcast mode to find the path in the WSN.
// *			  This message is very similar to the actual AODV protocol 
// * 			  implementation.
// **************************************************************************
// * part of:     EMoS - Elder Monitoring System
// **************************************************************************/

cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteRequest extends NetwPkt
{
	int 				RREQ_ID;			//RREQ ID
	LAddress::L3Type	finalDestAddr;		//final destination node
	int 				finalDestSeqNo; 	//destination sequence number
	LAddress::L3Type	initialSrcAddr;		//initial source node
	int 				initialSrcSeqNo; 	//source sequence number
	int					hopCount;			//hop count
}