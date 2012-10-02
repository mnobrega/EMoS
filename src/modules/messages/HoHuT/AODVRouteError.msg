cplusplus {{
    #include "SimpleAddress.h"
	#include "NetwPkt_m.h"
	#include <map>
	typedef std::map<LAddress::L3Type,int> addressSeqNoMap_t;
}}

class NetwPkt;
class noncobject addressSeqNoMap_t;

message AODVRouteError extends NetwPkt
{
	addressSeqNoMap_t	unreachDestAddresses;	//unreachable destination address
}