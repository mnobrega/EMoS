cplusplus {{
	#include "NetwPkt_m.h"
}}

class NetwPkt;

message AODVRouteRequest extends NetwPkt
{
	int 				RREQ_ID;	//RREQ ID
	int 				destSeqNo; 	//destination sequence number
	int 				srcSeqNo; 	//source sequence number
	int					hopCount;	//hop count
}