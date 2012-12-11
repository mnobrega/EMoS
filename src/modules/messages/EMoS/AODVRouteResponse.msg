//**************************************************************************
// * file:        AODVRouteResponse.msg
// *
// * author:      Márcio Nóbrega
// *
// * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
// *
// *           	  Message sent in unicast to inform the emiter that the receiver
// *			  has been reached and the path has been found. This message is
// *			  is sent to the emiter using the reverse path created when the
// * 			  AODVRouteRequest.msg was sent.
// **************************************************************************
// * part of:     EMoS - Elder Monitoring System
// **************************************************************************/

cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteResponse extends NetwPkt
{
	LAddress::L3Type	routeDestAddr;		//route destination addr
	LAddress::L3Type	routeSrcAddr;		//route source addr
	int 				routeDestSeqNo; 	//route destination seq number
	int					hopCount;			//hop count
}