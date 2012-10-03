cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVRouteError extends NetwPkt
{
	LAddress::L3Type	unreachDestAddress;		//unreachable destination address
	int 				unreadhDestAddrSeqNo;	//unreachable destination seqNo
}