cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message RERR extends NetwPkt
{
	int					destCount;	//the number of unreachable destinatinos
	LAddress::L3Type	destAddr;	//unreachable destination address
	int 				destSeqNo; 	//unreachable destination sequence number
}