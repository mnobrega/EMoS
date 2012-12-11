//**************************************************************************
// * file:        AODVRouteError.msg
// *
// * author:      Márcio Nóbrega
// *
// * copyright:   (C) 2012 IST - Instituto Superior Técnico, Lisbon, Portugal
// *
// *           	  Message used to inform the emiter and all the precursors
// * 			  that a path is no longer valid. 
// **************************************************************************
// * part of:     EMoS - Elder Monitoring System
// **************************************************************************/

cplusplus {{
	#include "NetwPkt_m.h"
	typedef std::map<LAddress::L3Type,int> addressSeqNoMap_t;
}}

class NetwPkt;
class noncobject addressSeqNoMap_t;

message AODVRouteError extends NetwPkt
{
	addressSeqNoMap_t		unreachDestAddress;		//unreachable destination address
}