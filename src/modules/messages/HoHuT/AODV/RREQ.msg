cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message RREQ extends NetwPkt
{
	int 				RREQ_ID;	//RREQ ID
	LAddress::L3Type 	destAddr;	//destination address
	int 				destSeqNo; 	//destination sequence number
	LAddress::L3Type	origAddr;	//originator address
	int 				origSeqNo; 	//originator sequence number
	int					hopCount;	//hop count
}