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