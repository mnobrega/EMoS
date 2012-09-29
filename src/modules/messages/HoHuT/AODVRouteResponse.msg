cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteResponse extends NetwPkt
{
	LAddress::L3Type	initialSrcAddr;		//initial source node
	LAddress::L3Type	finalDestAddr;		//final destination node
	int 				destSeqNo; 			//destination sequence number
	int					lifetime;			//time in milliseconds
	int					hopCount;			//hop count
}