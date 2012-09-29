cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteRequest extends NetwPkt
{
	int 				RREQ_ID;			//RREQ ID
	LAddress::L3Type	finalDestAddr;		//final destination node
	int 				destSeqNo; 			//destination sequence number
	LAddress::L3Type	initialSrcAddr;		//initial source node
	int 				srcSeqNo; 			//source sequence number
	int					hopCount;			//hop count
}