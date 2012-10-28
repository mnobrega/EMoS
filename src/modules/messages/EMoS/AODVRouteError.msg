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