cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message AODVData extends NetwPkt
{
	LAddress::L3Type		finalDestAddr;		//final destination node
	LAddress::L3Type		initialSrcAddr;		//initial source node
}