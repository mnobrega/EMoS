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