cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteError extends NetwPkt
{
	int					destCount;			//the number of unreachable destinations
	LAddress::L3Type	unreachDestAddr;	//unreachable destination address
	int 				unreachDestSeqNo; 	//unreachable destination sequence number
}