cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message RREP extends NetwPkt
{
	LAddress::L3Type 	destAddr;	//destination address
	int 				destSeqNo; 	//destination sequence number
	LAddress::L3Type	origAddr;	//originator address
	int					lifetime;	//time in milliseconds
	int					hopCount;	//hop count
}