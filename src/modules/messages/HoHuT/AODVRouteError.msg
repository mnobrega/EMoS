cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteError extends NetwPkt
{
	int					destCount;			//the number of unreachable destinations
	string				unreachDestAddr;	//unreachable destination address
}